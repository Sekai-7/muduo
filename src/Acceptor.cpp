
#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Logger.h"
#include "Channel.h"

static int createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr, bool reuseport) 
:mainloop(loop), accept_Socket(createNonblocking()), accept_channel(loop, accept_Socket.fd()), listening(false) 
{
    // 调用bind函数 
    accept_Socket.bindAddress(addr);
    accept_Socket.setReuseAddr(true);
    accept_Socket.setReusePort(true);
    // 设置新连接到来的回调
    accept_channel.setReadEventCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    accept_channel.disableReading();
    accept_channel.disableWriting();
    accept_channel.remove();
}

void Acceptor::listen() {
    accept_channel.enableReading();
    LOG_INFO("Start listening\n");
    accept_Socket.listen();
    listening = true;
}
    
bool Acceptor::islistening() {
    return listening;
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb) {
    this->cb = cb;
}

void Acceptor::handleRead() {
    // LOG_INFO("Acceptor::handleRead\n");
    InetAddress peer_addr;
    int accept_fd = accept_Socket.accept(&peer_addr);
    if (accept_fd >= 0) {
        LOG_INFO("Accept Connection!\n");
        if (cb)
            cb(accept_fd, peer_addr);
        else
            ::close(accept_fd);
    } else {
        LOG_ERROR("Acceptor Error!\n");
    }
}