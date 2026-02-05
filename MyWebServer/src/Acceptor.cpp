// Acceptor.cpp - 监听连接的实现
#include "../include/Acceptor.h"
#include "../include/EventLoop.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <functional>
#include <cstdio>

Acceptor::Acceptor(EventLoop* loop, const InetAddress &listenAddr)
    : loop_(loop),
      listenSocket_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)),
      listenChannel_(loop->getFd(), listenSocket_.fd()),
      listening_(false)
{
    listenSocket_.setReuseAddr(true);
    listenSocket_.bindAddress(listenAddr);
    listenChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
}

void Acceptor::listen() {
    listening_ = true;
    listenSocket_.listen();
    listenChannel_.enableReading();
}

void Acceptor::handleRead() {
    while (true) {
        InetAddress peerAddr;
        int connfd = listenSocket_.accept(&peerAddr);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("accept");
            break;
        }
        if (connfd >= 0) {
            if (newConnectionCallback_) {
                newConnectionCallback_(connfd, peerAddr);
            } else {
                ::close(connfd);
            }
        }
    }
}
