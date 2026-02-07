// TcpConnection.cpp - 支持多线程的 TCP 连接实现
#include "../include/TcpConnection.h"
#include "../include/EventLoop.h"
#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

// 默认连接空闲超时时间（秒）
static constexpr double kDefaultIdleTimeout = 20.0;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop->getFd(), sockfd)),
      peerAddr_(peerAddr)
{
    socket_->setNonBlock();
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
}

TcpConnection::~TcpConnection() {
    std::cout << "TcpConnection::dtor[" << name_ << "] fd=" << socket_->fd() << std::endl;
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->enableReading();
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
    // 建立连接时添加一个一次性超时定时器（若需要自动断开空闲连接）
    auto weak = weak_from_this();
    timerId_ = loop_->addTimer([weak]() {
        if (auto conn = weak.lock()) {
            // 在 loop 线程中触发完整关闭流程
            conn->forceClose();
        }
    }, addTime(Timestamp::now(), kDefaultIdleTimeout), 0.0);
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->removeFromEpoll();
        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    channel_->removeFromEpoll();
}

void TcpConnection::handleRead() {
    auto guard = shared_from_this();
    ssize_t n = inputBuffer_.read(socket_->fd());
    
    if (n > 0) {
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
        // 有数据到达，重置空闲超时定时器
        resetConnectionTimer();
    } else if (n == 0) {
        handleClose();
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("TcpConnection::handleRead");
            handleError();
        }
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        ssize_t n = ::send(socket_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes(), 0);
        if (n > 0) {
            outputBuffer_.retrecv(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
    }
}

void TcpConnection::handleClose() {
    std::cout << "TcpConnection::handleClose [" << name_ << "] fd=" << socket_->fd() << std::endl;
    setState(kDisconnected);
    channel_->removeFromEpoll();
    
    auto guardThis = shared_from_this();
    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }
    if (closeCallback_) {
        closeCallback_(socket_->fd());
    }
    // 连接关闭时取消关联的定时器
    cancelConnectionTimer();
}

void TcpConnection::handleError() {
    // 错误处理
    perror("TcpConnection::handleError");
}

void TcpConnection::send(const std::string &msg) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(msg);
        } else {
            // 跨线程发送，需要通过 runInLoop
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, msg));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& msg) {
    ssize_t nwrote = 0;
    size_t remaining = msg.size();
    
    // 如果没有在写并且输出缓冲区为空，尝试直接发送
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::send(socket_->fd(), msg.data(), msg.size(), 0);
        if (nwrote >= 0) {
            remaining = msg.size() - nwrote;
        } else {
            if (errno != EWOULDBLOCK) {
                perror("TcpConnection::sendInLoop");
            }
            nwrote = 0;
        }
    }
    
    // 如果还有剩余数据，放入输出缓冲区
    if (remaining > 0) {
        outputBuffer_.write(msg.data() + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::resetConnectionTimer() {
    // 先取消已有定时器（如果存在），然后添加新的超时定时器
    if (timerId_) {
        loop_->cancelTimer(timerId_);
        timerId_ = nullptr;
    }
    auto weak = weak_from_this();
    timerId_ = loop_->addTimer([weak]() {
        if (auto conn = weak.lock()) {
            conn->forceClose();
        }
    }, addTime(Timestamp::now(), kDefaultIdleTimeout), 0.0);
}

void TcpConnection::forceClose() {
    // 在所属 EventLoop 线程上执行完整关闭流程
    if (state_ == kConnected || state_ == kDisconnecting) {
        loop_->runInLoop(std::bind(&TcpConnection::handleClose, this));
    }
}

void TcpConnection::cancelConnectionTimer() {
    if (timerId_) {
        loop_->cancelTimer(timerId_);
        timerId_ = nullptr;
    }
}
