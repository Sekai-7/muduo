#pragma once

#include <arpa/inet.h>
#include <string>

// 可以用端口加ip的方式构造，也可用sockaddr_in构造
// 包含解析出ip，端口和ip加端口的三个函数
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in &addr): addr(addr){}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in *getSockAddr() const { return &addr; }
    void setSockAddr(const sockaddr_in &addr_temp) { addr = addr_temp; }

private:
    sockaddr_in addr;
};

