// Channel.cpp - implementations
#include "../include/Channel.h"
#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <cstdio>

Channel::Channel(int epfd, int fd)
    : epfd_(epfd), fd_(fd), events_(0), revents_(0), inEpoll_(false) {}

Channel::~Channel()
{
    if (inEpoll_)
    {
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd_, nullptr) == -1)
            perror("epoll_ctl DEL");
    }
}

void Channel::setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }

void Channel::enableReading()
{
    events_ |= EPOLLIN | EPOLLET;
    update();
}

void Channel::removeFromEpoll()
{
    if (inEpoll_)
    {
        epoll_ctl(epfd_, EPOLL_CTL_DEL, fd_, nullptr);
        inEpoll_ = false;
    }
}

void Channel::setRevents(int rev) { revents_ = rev; }

void Channel::handleEvent()
{
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
            readCallback_();
    }
}

void Channel::update()
{
    memset(&ev, 0, sizeof(ev));
    ev.events = events_;
    ev.data.ptr = this;

    if (!inEpoll_)
    {
        bool ok = (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &ev) == 0);
        if (ok)
            inEpoll_ = true;
        else
            perror("epoll_ctl ADD");
    }
    else
    {
        if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd_, &ev) == -1)
            perror("epoll_ctl MOD");
    }
}