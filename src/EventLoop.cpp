#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>


#include "EventLoop.h"
#include "Poller.h"
#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "Timestamp.h"

// 每个线程有一个，标识当前线程是否已存在loop
// 线程局部存储(Thread Local Storage，TLS)是一种存储期(storage time)，
// 对象的存储是在线程开始时分配，线程结束时回收，
// 每个线程有该对象自己的实例；
// 如果类的成员函数内定义了 thread_local 变量，
// 则对于同一个线程内的该类的多个对象都会共享一个变量实例，
// 并且只会在第一次执行这个成员函数时初始化这个变量实例。
thread_local EventLoop* t_loopInThisThread = nullptr;

const int time_out = 10000;

static int createEventfd() {
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0)
        LOG_FATAL("EventFd Fail!\n");
    else
        return event_fd;
}

// 调用eventloopthreadpool的start之后，在其中调用eventloopthread的start,通过启动线程和调用线程函数初始化EventLOop（栈对象）
EventLoop::EventLoop():looping(false), quiting(false), poller(Poller::newDefaultPoller(this)),
    wake_fd(createEventfd()), wake_channel(new Channel(this, wake_fd)), 
    thread_id(CurrentThread::tid()), calling_pending_fun(false)
{
    if (t_loopInThisThread == nullptr)
        t_loopInThisThread = this;
    else
        LOG_FATAL("Exist Loop!\n");
    wake_channel->setReadEventCallback(std::bind(&EventLoop::handleRead, this));
    wake_channel->enableReading();
}

EventLoop::~EventLoop() {
    wake_channel->disableReading();
    wake_channel->disableWriting();
    // 移除监听
    wake_channel->remove();
    quiting = true;
    close(wake_fd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping = true;
    while (quiting == false) {
        active_channel.clear();
        Timestamp time = poller->poll(time_out, active_channel);
        // LOG_INFO("Loop\n");
        for (auto c : active_channel) {
            // LOG_INFO("EventLoop handle\n");
            c->handleEvent(time);
        }
        doFunction();
    }
    looping = false;
}

// 这里调用的时候，其实就是想执行回调函数了
void EventLoop::runInLoop (Func cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
    return;
}

void EventLoop::queueInLoop (Func cb) {
    {
        std::unique_lock<std::mutex> ul(mutex);
        pending_fun.emplace_back(cb);
    }
    // 这边有个问题
    // 为什么要有检测条件呢
    // 其实还是和quit差不多，如果你能走到这里（在本线程中）
    // 那肯定没有被epollwait卡住，就不用wake
    // 那为什么是这两个条件呢，第一个显而易见
    // 第二个是因为如果线程正在执行回调函数，那需要再次唤醒它
    // 等这次执行完了后，下次立刻唤醒继续执行回调函数
    if (!isInLoopThread() || calling_pending_fun)
        wakeup();
    return;
}

void EventLoop::quit() {
    quiting = true; 
    // 为什么在本线程就不用wakeup呢？
    // 在本线程能调用到quit，说明肯定没有阻塞在epollwait
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::doFunction() {
    std::vector<Func> save;
    calling_pending_fun = true;
    {
        std::unique_lock<std::mutex> ul(mutex);
        pending_fun.swap(save);
    }
    for (auto fun : save)
        fun();
    calling_pending_fun = false;
    return;
}

void EventLoop::updateChannel(Channel* channel) {
    LOG_INFO("EventLoop::updateChannel\n");
    poller->updateChannel(channel);
    return;
}

void EventLoop::removeChannel(Channel* channel) {
    poller->removeChannel(channel);
    return;
}

bool EventLoop::hasChannel(Channel* channel) {
    return poller->hasChannel(channel);
}

bool EventLoop::isInLoopThread() const {
    return thread_id == CurrentThread::tid();
}

void EventLoop::handleRead() {
    int64_t temp = 1;
    int size = read(wake_fd, &temp, sizeof(temp));
    if (size <= 0)
        LOG_FATAL("Unable to Read!\n");
    LOG_INFO("Success Wakeup!\n");
    return;
}

void EventLoop::wakeup() {
    int64_t temp = 1;
    // eventfd不能写0
    int size = write(wake_fd, &temp, sizeof(temp));
    if (size <= 0)
        LOG_FATAL("Unable to Write!\n");
    return;
}