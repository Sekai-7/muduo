#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <string>

#include "Callbacks.h"

class EventLoop;
class InetAddress;
class Acceptor;
class EventLoopThreadPool;

class TcpServer {
public:
    enum Option {
        kNoReusePort,
        kReusePort,
    };

    using ThreadInitCallback = std::function<void(EventLoop*)>;
    TcpServer(EventLoop*, const InetAddress&, const std::string&, Option = kReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback&);
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCallback&);

    void setThreadNum(const int&);

    void start();
private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>; 

    void newConnection(const int&, const InetAddress&);
    void removeConnection(const TcpConnectionPtr&);
    void removeConnectionInLoop(const TcpConnectionPtr&);

    EventLoop* mainloop;

    const std::string ip_port;
    const std::string name;

    std::unique_ptr<Acceptor> acceptor;
    // 为什么是sharedptr呢？
    std::shared_ptr<EventLoopThreadPool> thread_pool;
    // 线程数量
    int thread_num;
    // 这个没啥用，只是给线程起名字
    int next_index;

    // 回调函数设置
    ConnectionCallback ccb;
    MessageCallback mcb;
    WriteCallback wcb;
    ThreadInitCallback ticb;

    std::atomic<int> started;

    ConnectionMap connection_map;
};

