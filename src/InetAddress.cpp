#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <cstring>

#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr(ip.c_str());
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    addr.sin_port = htons(port);
}
std::string InetAddress::toIp() const {
    char buffer[128] = {0};
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, buffer, 128);
    return buffer;
}
std::string InetAddress::toIpPort() const {
    return std::string (this->toIp() + ":" + std::to_string(toPort())); 
}

uint16_t InetAddress::toPort() const {
    return ntohs(addr.sin_port);
}

// int main() {
//     InetAddress addr;
//     std::cout << addr.toIp() << std::endl;
//     std::cout << addr.toPort() << std::endl;
//     std::cout << addr.toIpPort() << std::endl;
// }