// EventLoop.cpp - 支持多线程的事件循环实现
#include "../include/EventLoop.h"
#include "../include/Channel.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <cassert>
#include "../include/TimerQueue.h"

// 创建用于唤醒的 eventfd
static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        perror("Failed to create eventfd");
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : epfd_(epoll_create1(0)),
      events_(16),
      quit_(false),
      threadId_(std::this_thread::get_id()),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(epfd_, wakeupFd_)),
      timerQueue_(new TimerQueue(this)),
      callingPendingFunctors_(false)
{
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->removeFromEpoll();
    ::close(wakeupFd_);
    if (epfd_ >= 0)
        ::close(epfd_);
}



void EventLoop::loop() {
    quit_ = false;
    while (!quit_) {
        int n = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), -1);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        
        
        for (int i = 0; i < n; ++i) {
            Channel* activeChannel = static_cast<Channel*>(events_[i].data.ptr);
            activeChannel->setRevents(events_[i].events);
            activeChannel->handleEvent();
        }
        
        
        if (static_cast<size_t>(n) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
        
        
        doPendingFunctors();
    }
}

void EventLoop::quit() {
    quit_ = true;
   
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    
    
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("EventLoop::wakeup() writes");
    }
}

void EventLoop::handleWakeup() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("EventLoop::handleWakeup() reads");
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    
    for (const auto& functor : functors) {
        functor();
    }
    
    callingPendingFunctors_ = false;
}

// 将定时器添加到 EventLoop 的 TimerQueue 中，返回 TimerId
TimerQueue::TimerId EventLoop::addTimer(TimerQueue::TimerCallback cb, Timestamp when, double interval) {
    if (timerQueue_) {
        return timerQueue_->addTimer(std::move(cb), when, interval);
    }
    return nullptr;
}

// 取消定时器
void EventLoop::cancelTimer(TimerQueue::TimerId id) {
    if (timerQueue_ && id) {
        timerQueue_->cancel(id);
    }
}

void EventLoop::updateChannel(Channel* channel) {
    
}

void EventLoop::removeChannel(Channel* channel) {
    
}
