#pragma once

#include <sys/socket.h>
#include <unistd.h>

#include "InetAddress.h"

// 管理sockfd的生命周期

class Socket
{
public:
    Socket(const int& fd):sockfd(fd) {}
    ~Socket() {
        close(sockfd);
    }

    int fd() const;
    void bindAddress(const InetAddress&);
    void listen();
    int accept(InetAddress* perraddr);

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    void shutdownWrite();

private:
    const int sockfd;
};

