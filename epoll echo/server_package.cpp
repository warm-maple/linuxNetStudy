#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <memory>

class InetAddress
{
public:
    struct sockaddr_in addr_;
    InetAddress() { memset(&addr_, 0, sizeof(addr_)); }
    InetAddress(int port)
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_.sin_port = htons(port);
    }
    InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}
};

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel(int epfd, int fd)
        : epfd_(epfd), fd_(fd), events_(0), revents_(0), inEpoll_(false) {}

    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }

    void enableReading()
    {
        events_ |= EPOLLIN | EPOLLET;
        update();
    }

    void setRevents(int rev) { revents_ = rev; }

    void handleEvent()
    {
        if (revents_ & (EPOLLIN | EPOLLPRI))
        {
            if (readCallback_)
                readCallback_();
        }
    }

private:
    void update()
    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = events_;
        ev.data.ptr = this;

        if (!inEpoll_)
        {
            epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &ev);
            inEpoll_ = true;
        }
        else
        {
            epoll_ctl(epfd_, EPOLL_CTL_MOD, fd_, &ev);
        }
    }

    int epfd_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool inEpoll_;
    EventCallback readCallback_;
};

class Socket
{
public:
    explicit Socket(int fd) : fd_(fd) {}
    ~Socket()
    {
        if (fd_ >= 0)
            close(fd_);
    }

    int fd() const { return fd_; }

    void bindAddress(const InetAddress &localaddr)
    {
        bind(fd_, (struct sockaddr *)&localaddr.addr_, sizeof(localaddr.addr_));
    }
    void listen() { ::listen(fd_, 128); }

    int accept(InetAddress *peeraddr)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int connfd = ::accept(fd_, (struct sockaddr *)&addr, &len);
        if (connfd >= 0)
        {
            peeraddr->addr_ = addr;
        }
        return connfd;
    }

    void setNonBlock()
    {
        int flags = fcntl(fd_, F_GETFL, 0);
        fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    }

    void setReuseAddr(bool on)
    {
        int opt = on ? 1 : 0;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

private:
    int fd_;
};
class Buffer
{
public:
    
    Buffer() { buff.reserve(8192); }
    size_t size() { return buff.size(); }
    void buffclear(){
        buff.clear();
    }
    auto getMes()
    {
        return buff.data();
    }
   int read(int client_fd)
    {
        ssize_t read_len;
        while (true)
        {       char temp_buf[1024];
            memset(temp_buf, 0, sizeof(temp_buf));
            read_len = recv(client_fd, temp_buf, 1023, 0);
            if (read_len > 0)
            {
                buff.insert(buff.end(), temp_buf, temp_buf + read_len);
            }
            else if (read_len == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                return -1;
            }
            else if (read_len == 0)
            {
                return 0;
            }
        }return buff.size();
        
    }

private:
    std::vector<char> buff;

};
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using CloseCallback = std::function<void(int)>;
    TcpConnection(int epfd, int sockfd)
        : socket_(new Socket(sockfd)),
          channel_(new Channel(epfd, sockfd)), buff()
    {
        socket_->setNonBlock();
        channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
        channel_->enableReading();
    }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

private:
    void handleClose()
    {
        std::cout << "客户端(" << socket_->fd() << ")断开连接" << std::endl;
        if (closeCallback_)
        {
            closeCallback_(socket_->fd());
        }
    }
    void handleRead()
    {   auto guard = shared_from_this();
        ssize_t n = buff.read(socket_->fd());
        if (n > 0)
        {
            std::cout << "收到客户端(" << socket_->fd() << ")消息: " << buff.getMes() << std::endl;
            write(socket_->fd(), buff.getMes(), n);
            buff.buffclear();
        }
        else if (n == 0)
        {
            std::cout << "客户端(" << socket_->fd() << ")断开连接" << std::endl;
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

    std::unique_ptr<Socket> socket_;
    CloseCallback closeCallback_;
    std::unique_ptr<Channel> channel_;
    Buffer buff;
};

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

    Acceptor(int epfd, const InetAddress &listenAddr)
        : listenSocket_(socket(AF_INET, SOCK_STREAM, 0)),
          listenChannel_(epfd, listenSocket_.fd())
    {
        listenSocket_.setReuseAddr(true);
        listenSocket_.bindAddress(listenAddr);
        listenSocket_.listen();

        listenChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
        listenChannel_.enableReading();
    }

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

private:
    void handleRead()
    {
        InetAddress peerAddr;
        int connfd = listenSocket_.accept(&peerAddr);
        if (connfd >= 0)
        {
            if (newConnectionCallback_)
            {
                newConnectionCallback_(connfd, peerAddr);
            }
        }
    }

    Socket listenSocket_;
    Channel listenChannel_;
    NewConnectionCallback newConnectionCallback_;
};
class EventLoop
{
public:
    ~EventLoop() { close(epfd); }
    EventLoop() : epfd(epoll_create1(0)), events(100)
    {
    }
    int getFd()
    {
        return epfd;
    }
    void loop()
    {
        while (true)
        {
            int n = epoll_wait(epfd, events.data(), 100, -1);

            for (int i = 0; i < n; ++i)
            {
                Channel *activeChannel = static_cast<Channel *>(events[i].data.ptr);
                activeChannel->setRevents(events[i].events);
                activeChannel->handleEvent();
            }
        }
    }

private:
    int epfd;
    std::vector<struct epoll_event> events;
};
class TcpServer
{
public:
    TcpServer(EventLoop &loop, InetAddress addr) : loop(loop), acceptor(loop.getFd(), addr)
    {
        acceptor.setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    void removeConnection(int connfd)
    {
        std::cout << "TcpServer 移除连接 FD=" << connfd << std::endl;
        connections.erase(connfd);
    }
    void newConnection(int connfd, const InetAddress &addr)
    {

        std::cout << "新连接建立！FD = " << connfd << std::endl;
        auto conn = std::make_shared<TcpConnection>(loop.getFd(), connfd);
        conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        connections[connfd] = conn;
    }
    EventLoop &loop;
    Acceptor acceptor;
    std::map<int, std::shared_ptr<TcpConnection>> connections;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8888);
    TcpServer server(loop, addr);
    loop.loop();
    return 0;
}