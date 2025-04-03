#pragma once

#include <functional>
#include <condition_variable>

#include "noncopyable.h"
#include "Thread.h"

class EventLoop;

class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback&, const std::string& = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    ThreadInitCallback callback;
    void threadFunc();
    EventLoop* loop;
    bool existing;
    Thread thread;
    std::mutex mutex;
    std::condition_variable cond;
};

