#include <netinet/tcp.h>
#include <cstring>

#include "Socket.h"
#include "InetAddress.h"

int Socket::fd() const {
    return sockfd;
}

void Socket::bindAddress(const InetAddress& address) {
    bind(sockfd, (sockaddr*)address.getSockAddr(), sizeof(sockaddr_in));
    return;
}

int Socket::accept(InetAddress* perrraddr) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t size = sizeof(addr);
    int fd = accept4(sockfd, (sockaddr*)&addr, &size, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd >= 0)
        perrraddr->setSockAddr(addr);
    return fd;
}

void Socket::listen() {
    ::listen(sockfd, 10);
    return;
}

void Socket::shutdownWrite() {
    // 关闭写端
    ::shutdown(sockfd, SHUT_WR);
}

void Socket::setTcpNoDelay(bool on)
{
    // TCP_NODELAY 用于禁用 Nagle 算法。
    // Nagle 算法用于减少网络上传输的小数据包数量。
    // 将 TCP_NODELAY 设置为 1 可以禁用该算法，允许小数据包立即发送。
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    // SO_REUSEADDR 允许一个套接字强制绑定到一个已被其他套接字使用的端口。
    // 这对于需要重启并绑定到相同端口的服务器应用程序非常有用。
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on)
{
    // SO_REUSEPORT 允许同一主机上的多个套接字绑定到相同的端口号。
    // 这对于在多个线程或进程之间负载均衡传入连接非常有用。
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    // SO_KEEPALIVE 启用在已连接的套接字上定期传输消息。
    // 如果另一端没有响应，则认为连接已断开并关闭。
    // 这对于检测网络中失效的对等方非常有用。
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}