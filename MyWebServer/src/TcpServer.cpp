#include "TcpServer.h"
#include <iostream>

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string& nameArg)
    : loop_(loop),
      ipPort_(addr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, addr)),
      threadPool_(new EventLoopThreadPool(loop, name_))
{
    
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, 
        std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto& item : connections_) {
        auto conn = item.second;
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}
void TcpServer::start() {
    threadPool_->start();
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = threadPool_->getNextLoop();
    std::string connName = name_ + "-" + ipPort_ + "#" + std::to_string(sockfd);
    std::cout << "TcpServer::newConnection [" << name_ << "] - new connection [" 
              << connName << "] from " << peerAddr.toIpPort() << std::endl;
    auto conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, peerAddr);
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    connections_[sockfd] = conn;
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(int connfd) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, connfd));
}

void TcpServer::removeConnectionInLoop(int connfd) {
    std::cout << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection fd:" << connfd << std::endl;
    
    auto it = connections_.find(connfd);
    if (it != connections_.end()) {
        auto conn = it->second;
        connections_.erase(it);
        
        EventLoop* ioLoop = conn->getLoop();
        ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}