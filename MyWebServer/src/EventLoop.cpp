// EventLoop.cpp - implementations
#include "../include/EventLoop.h"
#include "../include/Channel.h"
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>

EventLoop::EventLoop() : epfd(epoll_create1(0)), events(100) {}

EventLoop::~EventLoop()
{
    if (epfd >= 0)
        close(epfd);
}

int EventLoop::getFd() { return epfd; }

void EventLoop::loop()
{
    while (true)
    {
        int n = epoll_wait(epfd, events.data(), static_cast<int>(events.size()), -1);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i)
        {
            Channel *activeChannel = static_cast<Channel *>(events[i].data.ptr);
            activeChannel->setRevents(events[i].events);
            activeChannel->handleEvent();
        }
    }
}