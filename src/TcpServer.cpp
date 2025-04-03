#include <unordered_map>
#include <memory>
#include <functional>
#include <cstring>

#include "TcpServer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Buffer.h"
#include "TcpConnection.h"

static EventLoop* checkLoop(EventLoop* loop) {
    if (loop == nullptr)
        LOG_FATAL("MainLoop is nullptr\n");
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr, const std::string& name, Option option) 
    :mainloop(checkLoop(loop)), ip_port(addr.toIpPort()), name(name),
    acceptor(new Acceptor(loop, addr, option)), thread_pool(new EventLoopThreadPool(loop, name)),
    thread_num(0), next_index(0), started(0)
{
    // 设置新连接到来的回调，实际就是选择一个线程管理新连接
    // 会被acceptor的channel在读事件触发时调用
    acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto item : connection_map) {
        TcpConnectionPtr conn(item.second); 
        item.second.reset();
        mainloop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback& ticb) {
    this->ticb = ticb;
}

void TcpServer::setConnectionCallback(const ConnectionCallback& ccb) {
    this->ccb = ccb;
}

void TcpServer::setMessageCallback(const MessageCallback& mcb) {
    this->mcb = mcb;
}

void TcpServer::setWriteCompleteCallback(const WriteCallback& wcb) {
    this->wcb = wcb;
}

void TcpServer::setThreadNum(const int& count) {
    thread_num = count;
    thread_pool->setThreadNum(thread_num);
}

void TcpServer::start() {
    if (started.fetch_add(1) == 0) {
        thread_pool->start(ticb);
        // mainloop的开始loop是用户调用的
        mainloop->runInLoop(std::bind(&Acceptor::listen, acceptor.get()));
    } 
    return;
}

void TcpServer::newConnection(const int& fd, const InetAddress& peer_addr) {
    EventLoop* now_loop = thread_pool->getNextLoop();
    std::string conn_name = name + " " + ip_port.c_str() + " " + std::to_string(next_index);   
    ++next_index;
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    socklen_t len = sizeof(local_addr);
    if (::getsockname(fd, (sockaddr*)&local_addr, &len) == -1) {
        LOG_FATAL("GetSockName Error!\n");
    }
    TcpConnectionPtr conn(new TcpConnection(now_loop, conn_name, fd, InetAddress(local_addr), peer_addr));
    connection_map[name] = conn;
    conn->setMessageCallback(mcb);
    conn->setConnectionCallback(ccb);
    conn->setWriteCompleteCallback(wcb);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));
    now_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    return;
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    mainloop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    connection_map.erase(conn->getName());
    EventLoop* loop = conn->getLoop();
    loop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn.get()));
}