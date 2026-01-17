// InetAddress.cpp - implementations
#include "../include/InetAddress.h"
#include <cstring>
#include <arpa/inet.h>

InetAddress::InetAddress() { memset(&addr_, 0, sizeof(addr_)); }

InetAddress::InetAddress(int port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

