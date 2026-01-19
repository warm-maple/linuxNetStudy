// Channel.h - declaration only
#pragma once

#include <sys/epoll.h>
#include <functional>
#include <cstdint>

class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(int epfd, int fd);
    ~Channel();

    void setReadCallback(EventCallback cb);
    void enableReading();
    void removeFromEpoll();
    void setRevents(int rev);
    void handleEvent();
    void enableWriting();
    void disableWriting();
    bool isWriting() const;
    void setWriteCallback(EventCallback cb);

private:
    void update();

    int epfd_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool inEpoll_;
    EventCallback readCallback_;
    struct epoll_event ev;
    EventCallback writeCallback_;
};