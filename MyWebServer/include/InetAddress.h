// InetAddress.h - 网络地址封装
#pragma once

#include <netinet/in.h>
#include <string>

class InetAddress {
public:
    InetAddress();
    explicit InetAddress(int port);
    InetAddress(const struct sockaddr_in &addr);

    const struct sockaddr* getSockAddr() const {
        return reinterpret_cast<const struct sockaddr*>(&addr_);
    }
    
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

    struct sockaddr_in addr_;
};