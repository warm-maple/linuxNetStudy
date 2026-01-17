// Acceptor.cpp - implementations
#include "../include/Acceptor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <functional>
#include <cstdio>

Acceptor::Acceptor(int epfd, const InetAddress &listenAddr)
    : listenSocket_(::socket(AF_INET, SOCK_STREAM, 0)), listenChannel_(epfd, listenSocket_.fd())
{
    listenSocket_.setReuseAddr(true);
    listenSocket_.bindAddress(listenAddr);
    listenSocket_.listen();
    listenSocket_.setNonBlock();
    listenChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
    listenChannel_.enableReading();
}

Acceptor::~Acceptor() {}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }

void Acceptor::handleRead()
{
    while (true)
    {
        InetAddress peerAddr;
        int connfd = listenSocket_.accept(&peerAddr);
        if (connfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("accept");
            break;
        }
        if (connfd >= 0)
        {
            if (newConnectionCallback_)
            {
                newConnectionCallback_(connfd, peerAddr);
            }
        }
    }
}
