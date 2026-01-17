// Acceptor.h - declaration only
#pragma once

#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

    Acceptor(int epfd, const InetAddress &listenAddr);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb);

private:
    void handleRead();

    Socket listenSocket_;
    Channel listenChannel_;
    NewConnectionCallback newConnectionCallback_;
};