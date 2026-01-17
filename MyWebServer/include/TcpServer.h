// TcpServer.h - declaration only
#pragma once

#include "EventLoop.h"
#include "InetAddress.h"
#include <memory>
#include "Acceptor.h"
#include "TcpConnection.h"
#include <map>

class TcpServer {
public:
    TcpServer(EventLoop &loop, InetAddress addr);
    ~TcpServer();

private:
    void removeConnection(int connfd);
    void newConnection(int connfd, const InetAddress &addr);

    EventLoop &loop;
    Acceptor acceptor;
    std::map<int, std::shared_ptr<TcpConnection>> connections;
};