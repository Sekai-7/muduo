#pragma once

#include <functional>

#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;


class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop*, const InetAddress&, bool);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback&);
    bool islistening();

    void listen();

private:
    // 处理新用户的连接事件
    void handleRead();
    EventLoop* mainloop;
    Socket accept_Socket;
    Channel accept_channel;
    NewConnectionCallback cb;
    bool listening;
};

