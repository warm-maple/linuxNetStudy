// TcpServer.cpp - implementations
#include "../include/TcpServer.h"
#include <iostream>
#include <functional>

TcpServer::TcpServer(EventLoop &loop_, InetAddress addr) : loop(loop_), acceptor(loop.getFd(), addr)
{
    acceptor.setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::removeConnection(int connfd)
{
    std::cout << "TcpServer 移除连接 FD=" << connfd << std::endl;
    connections.erase(connfd);
}

void TcpServer::newConnection(int connfd, const InetAddress &addr)
{
    std::cout << "新连接建立！FD = " << connfd << std::endl;
    auto conn = std::make_shared<TcpConnection>(loop.getFd(), connfd);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    connections[connfd] = conn;
}
