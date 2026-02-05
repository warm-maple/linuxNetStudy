// TcpConnection.h - 支持多线程的 TCP 连接
#pragma once

#include "Buffer.h"
#include "InetAddress.h"
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include "Channel.h"
#include "Socket.h"
#include "HttpContext.h"

class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback = std::function<void(int)>;

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    // 发送数据
    void send(const std::string &msg);
    void shutdown();

    // 设置回调
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    // 连接建立时调用（由 TcpServer 通过 runInLoop 调用）
    void connectEstablished();
    // 连接销毁时调用
    void connectDestroyed();

    HttpContext& getContext() { return context_; }

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    
    void sendInLoop(const std::string& msg);
    void shutdownInLoop();
    void setState(StateE s) { state_ = s; }

    EventLoop* loop_;
    std::string name_;
    std::atomic<StateE> state_;
    
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress peerAddr_;
    
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
    
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    HttpContext context_;
};