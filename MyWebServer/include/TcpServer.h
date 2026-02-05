#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <functional>
#include <string>
#include <memory>
#include <map>

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h" 
class TcpServer {
public:
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>;
    using ConnectionCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;

    TcpServer(EventLoop *loop, const InetAddress &addr, const std::string& nameArg = "TcpServer");
    ~TcpServer();

    
    void setThreadNum(int numThreads);

    
    void start();

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(int connfd);
    void removeConnectionInLoop(int connfd);

    EventLoop *loop_;  
    std::string ipPort_;
    std::string name_;
    std::unique_ptr<Acceptor> acceptor_; 
    std::unique_ptr<EventLoopThreadPool> threadPool_; 
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    std::map<int, std::shared_ptr<TcpConnection>> connections_;
};

#endif