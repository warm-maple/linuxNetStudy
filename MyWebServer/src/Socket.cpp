// Socket.cpp - implementations
#include "../include/Socket.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>

void Socket::setnonlock(int client_fd)
{
    int old_op = fcntl(client_fd, F_GETFL);
    int new_op = old_op | O_NONBLOCK;
    fcntl(client_fd, F_SETFL, new_op);
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket()
{
    if (fd_ >= 0)
        close(fd_);
}

int Socket::fd() const { return fd_; }

void Socket::bindAddress(const InetAddress &localaddr)
{
    bind(fd_, reinterpret_cast<const struct sockaddr *>(&localaddr.addr_), sizeof(localaddr.addr_));
}

void Socket::listen() { ::listen(fd_, 128); }

int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connfd = ::accept(fd_, reinterpret_cast<struct sockaddr *>(&addr), &len);
    if (connfd >= 0)
    {
        peeraddr->addr_ = addr;
    }
    return connfd;
}

void Socket::setNonBlock()
{
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

void Socket::shutdownWrite()
{
    if (fd_ >= 0) {
        ::shutdown(fd_, SHUT_WR);
    }
}

void Socket::setReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}