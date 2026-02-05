// InetAddress.cpp - 网络地址实现
#include "../include/InetAddress.h"
#include <cstring>
#include <arpa/inet.h>

InetAddress::InetAddress() {
    memset(&addr_, 0, sizeof(addr_));
}

InetAddress::InetAddress(int port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = ::strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    snprintf(buf + end, sizeof(buf) - end, ":%u", port);
    return buf;
}

uint16_t InetAddress::port() const {
    return ntohs(addr_.sin_port);
}
