#pragma once


#include <unistd.h>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <string>

class Thread
{
public:
    using Func = std::function<void()>;
    explicit Thread(Func, const std::string& = "");
    ~Thread();
    void join();
    void start();
    bool isJoined();
    bool isStarted();
    pid_t getTid() const;
    const std::string getName() const;
private:
    void setDefaultName();
private:
    bool joined;
    bool started; 
    pid_t tid;
    std::string name;
    // 线程回调函数
    Func func;
    // 这边为什么用sharedptr不用uniqueptr呢
    std::shared_ptr<std::thread> thread;
    // 这边是计数线程数量
    static std::atomic<int> count;
};

