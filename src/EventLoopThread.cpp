#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Thread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    :callback(cb), loop(nullptr), existing(false), thread(std::bind(&EventLoopThread::threadFunc, this))
{
}

EventLoopThread::~EventLoopThread() {
    existing = true;
    if (loop != nullptr) {
        loop->quit();
        thread.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    thread.start();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [&]() {return loop != nullptr;});
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (callback)
        callback(&loop);
    {
        // 条件变量的通知必须在锁内
        std::unique_lock<std::mutex> lock(mutex);
        this->loop = &loop;
        cond.notify_one();
    }
    loop.loop();
    std::unique_lock<std::mutex> lock(mutex);
    this->loop = nullptr;
    return;
}
