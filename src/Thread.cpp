#include "semaphore.h"

#include <string>
#include <atomic>
#include <memory>
#include <thread>


#include "Thread.h"
#include "CurrentThread.h"

std::atomic<int> Thread::count(0);

Thread::Thread(Func func, const std::string& str):joined(false),
    started(false), tid(0), name(str), func(std::move(func))
{
    setDefaultName();
}

Thread::~Thread() {
    if (isStarted() && !isJoined()) {
        thread->detach();
    }
    return;
}

bool Thread::isJoined() {
    return joined;
}

bool Thread::isStarted() {
    return started;
}

pid_t Thread::getTid() const {
    return tid;
}

void Thread::join() {
    joined = true;
    thread->join();
    return;
}

void Thread::start() {
    started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread = std::shared_ptr<std::thread>(new std::thread([&]() {
        tid = CurrentThread::tid();
        sem_post(&sem);
        func();
    }));
    // 线程create之后就会开始执行，如果这里不阻塞
    // tid的值如果在后续使用的话可能就是错的
    sem_wait(&sem);
}

void Thread::setDefaultName() {
    int num = ++count;
    if (name == "") {
        name = "Thread";
        name += std::to_string(num);
    } 
    return;
}