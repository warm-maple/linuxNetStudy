// Socket.h - declaration only
#pragma once

#include "InetAddress.h"
#include <unistd.h>

class Socket {
public:
    explicit Socket(int fd);
    ~Socket();

    int fd() const;

    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);

    void setNonBlock();
    void setReuseAddr(bool on);

    // helper for external use
    static void setnonlock(int client_fd);

private:
    int fd_;
};