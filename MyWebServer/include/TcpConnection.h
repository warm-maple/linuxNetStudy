// TcpConnection.h - declaration only
#pragma once

#include "Buffer.h"
#include <functional>
#include <memory>
#include "Channel.h"
#include "Socket.h"

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using CloseCallback = std::function<void(int)>;
    TcpConnection(int epfd, int sockfd);
    ~TcpConnection();

    void setCloseCallback(const CloseCallback &cb);
    void send(const std::string &msg);
    void handleWrite();
private:
    void handleClose();
    void handleRead();

    std::unique_ptr<Socket> socket_;
    CloseCallback closeCallback_;
    std::unique_ptr<Channel> channel_;
    Buffer input_buff;
    Buffer output_buff;
};