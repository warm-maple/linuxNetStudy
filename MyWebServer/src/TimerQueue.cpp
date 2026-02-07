#include "../include/TimerQueue.h"
#include "../include/EventLoop.h"
#include "../include/Timer.h"
#include "../include/Timefd.h"
#include <unistd.h>
#include <iostream>

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimefd()),  
      timerfdChannel_(loop->getFd(), timerfd_),  
      timers_() 
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}
TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableWriting();
    timerfdChannel_.removeFromEpoll();
    ::close(timerfd_);
    for (const auto& entry : timers_) {
        delete entry.second;
    }
}
using TimerId = Timer*;
TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    // 立即分配 Timer 对象并返回句柄（Timer*）。
    // 实际插入 timers_ 在 loop 线程中执行以保证线程安全。
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop([this, timer]() {
        bool earliestChanged = false;
        auto it = timers_.begin();
        if (it == timers_.end() || timer->getexpiration() < it->first) {
            earliestChanged = true;
        }
        timers_.insert({timer->getexpiration(), timer});
        if (earliestChanged) {
             resetTimerfd(timerfd_, timer->getexpiration());
        }
    });
    return timer;
}
void TimerQueue::handleRead()
{
    Timestamp now = Timestamp::now();
    uint64_t howmany;
    ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
    if (n != sizeof(howmany)) {
        std::cerr << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
    }
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    for (const auto& it : expired) {
        it.second->run();
    }
    for (const auto& it : expired) {
        if (it.second->isRepeat()) {
            it.second->restart(now);
            timers_.insert({it.second->getexpiration(), it.second});
        } else {
            delete it.second;
        }
    }
    if (!timers_.empty()) {
        resetTimerfd(timerfd_, timers_.begin()->first);
    }
}


void TimerQueue::resetTimerfd(int timerfd, Timestamp expiration)
{
    
    ::resetTimerfd(timerfd, expiration);
}


    void TimerQueue::cancel(TimerId id)
    {
        if (!id) return;
        // 在 loop 线程中执行删除操作，保证线程安全
        loop_->runInLoop([this, id]() {
            // 在 timers_ 中查找所有匹配的 Timer* 并删除
            for (auto it = timers_.begin(); it != timers_.end(); ) {
                if (it->second == id) {
                    // 删除对象并从集合中移除
                    delete it->second;
                    it = timers_.erase(it);
                } else {
                    ++it;
                }
            }
            // 重新设置最近的定时器
            if (!timers_.empty()) {
                resetTimerfd(timerfd_, timers_.begin()->first);
            }
        });
    }
