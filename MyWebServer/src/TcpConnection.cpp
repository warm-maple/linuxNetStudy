// TcpConnection.cpp - implementations
#include "../include/TcpConnection.h"
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

TcpConnection::TcpConnection(int epfd, int sockfd)
    : socket_(new Socket(sockfd)), channel_(new Channel(epfd, sockfd)), buff()
{
    socket_->setNonBlock();
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->enableReading();
}

TcpConnection::~TcpConnection() {}

void TcpConnection::setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

void TcpConnection::handleClose()
{
    std::cout << "客户端(" << socket_->fd() << ")断开连接" << std::endl;
    channel_->removeFromEpoll();
    if (closeCallback_)
    {
        closeCallback_(socket_->fd());
    }
}

void TcpConnection::handleRead()
{
    auto guard = shared_from_this();
    ssize_t n = buff.read(socket_->fd());
    if (n > 0)
    {
        std::string msg = buff.getMes();
        std::cout << "收到客户端(" << socket_->fd() << ")消息: " << msg << std::endl;
        size_t sent = 0;
        while (sent < msg.size())
        {
            ssize_t s = ::send(socket_->fd(), msg.data() + sent, msg.size() - sent, 0);
            if (s > 0)
                sent += s;
            else if (s == -1 && errno == EINTR)
                continue;
            else
            {
                perror("send");
                break;
            }
        }
        buff.buffclear();
    }
    else if (n == 0)
    {
        std::cout << "客户端(" << socket_->fd() << ")断开连接" << std::endl;
        channel_->removeFromEpoll();
        handleClose();
        buff.buffclear();
    }
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("read error");
            handleClose();
        }
    }
}