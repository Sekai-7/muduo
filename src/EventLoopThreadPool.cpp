#include <vector>
#include <memory>
#include <functional>
#include <string>

#include "EventLoopThread.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, const std::string& name) 
    :mainloop(loop), name(name), started(false), num_thread(0), next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallBack& cb) {
    started = true;
    for (int i = 0; i < num_thread; ++i) {
        std::string temp = name + std::to_string(i);
        EventLoopThread* up = new EventLoopThread(cb, temp);
        thread.push_back(std::unique_ptr<EventLoopThread>(up));
        loop.push_back(up->startLoop());
    }
    // 只有主线程的话其实就用不到EventLoopThread了
    if (num_thread == 0) {
        cb(mainloop); 
    }
    return;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoop() {
    if (num_thread == 0)
        return std::vector<EventLoop*>(1, mainloop);
    else 
        return loop;
}

bool EventLoopThreadPool::isStarted() const {
    return started;
}

void EventLoopThreadPool::setThreadNum(const int& count) {
    num_thread = count;
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    if (num_thread == 0)
        return mainloop;
    EventLoop* temp = loop[next];
    ++next;
    if (next == loop.size())
        next = 0;
    return temp;
}