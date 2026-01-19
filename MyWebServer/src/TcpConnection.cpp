// TcpConnection.cpp - implementations
#include "../include/TcpConnection.h"
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

TcpConnection::TcpConnection(int epfd, int sockfd)
    : socket_(new Socket(sockfd)), channel_(new Channel(epfd, sockfd)), input_buff()
{
    socket_->setNonBlock();
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->enableReading();
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
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
    ssize_t n = input_buff.read(socket_->fd());
    std::cout << "收到客户端(" << socket_->fd() << ")消息" << std::endl;
    if (n > 0)
    {

        while (true)
        {
            std::string msg = input_buff.getMes();
            size_t pos = msg.find('\n');
            if (pos != std::string::npos)
            {
                size_t len = pos + 1; // length within the returned msg (which starts from unread offset)
                std::string one_msg = msg.substr(0, len);
                std::cout << "处理完整包: " << one_msg;
                this->send(one_msg);
                input_buff.retrecv(len);
            }
            else
            {
                break;
            }
        }
    }
    else if (n == 0)
    {
        std::cout << "客户端(" << socket_->fd() << ")断开连接" << std::endl;
        channel_->removeFromEpoll();
        handleClose();
        input_buff.buffclear();
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
void TcpConnection::send(const std::string &msg)
{
    size_t nwrote = 0;
    size_t remaining = msg.size();
    if (!channel_->isWriting() && output_buff.getReadLen() == 0) {
        nwrote = ::send(socket_->fd(), msg.data(), msg.size(), 0);
        if (nwrote >= 0) {
            remaining = msg.size() - nwrote;
        } else {
            if (errno != EWOULDBLOCK) perror("send error");
            nwrote = 0; 
        }
    }
    if (remaining > 0) {
        output_buff.write(msg.data() + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting(); 
        }
    }
}
void TcpConnection::handleWrite(){
    if (channel_->isWriting()) {
        ssize_t n = ::send(socket_->fd(), output_buff.data(), output_buff.getReadLen(), 0);
        if (n > 0) {
            output_buff.retrecv(n); 
            if (output_buff.getReadLen() == 0) {
                channel_->disableWriting();
            }
        }
    }
}
