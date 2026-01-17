// EventLoop.h - declaration only
#pragma once

#include "Channel.h"
#include <vector>

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    int getFd();
    void loop();

private:
    int epfd;
    std::vector<struct epoll_event> events;
};