// Acceptor.h - 监听连接的封装
#pragma once

#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;

class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

    Acceptor(EventLoop* loop, const InetAddress &listenAddr);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb);
    
    // 开始监听
    void listen();
    
    bool listening() const { return listening_; }

private:
    void handleRead();

    EventLoop* loop_;
    Socket listenSocket_;
    Channel listenChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};