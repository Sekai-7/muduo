#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "EventLoopThread.h"

class EventLoop;

class EventLoopThreadPool
{
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop*, const std::string& = "");
    ~EventLoopThreadPool();

    void start(const ThreadInitCallBack&);

    bool isStarted() const;

    void setThreadNum(const int&);

    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoop();

private:
    // 主loop
    EventLoop* mainloop;
    // 线程池名
    std::string name;
    // 标记是否开启
    bool started;
    // 线程数量
    int num_thread;
    // 下一个线程索引
    int next;
    std::vector<std::unique_ptr<EventLoopThread>> thread;
    std::vector<EventLoop*> loop;
};

