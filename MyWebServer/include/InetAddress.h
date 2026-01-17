// InetAddress.h - declaration only
#pragma once

#include <netinet/in.h>

class InetAddress {
public:
    InetAddress();
    explicit InetAddress(int port);
    InetAddress(const struct sockaddr_in &addr);

    struct sockaddr_in addr_;
};