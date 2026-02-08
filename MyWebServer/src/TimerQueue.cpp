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
    timerfdChannel_.removeFromEpoll();
    ::close(timerfd_);
    for (const auto& entry : timers_) {
        delete entry.second;
    }
}
using TimerId = Timer*;
TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{

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

        loop_->runInLoop([this, id]() {

            for (auto it = timers_.begin(); it != timers_.end(); ) {
                if (it->second == id) {
                    delete it->second;
                    it = timers_.erase(it);
                } else {
                    ++it;
                }
            }
            if (!timers_.empty()) {
                resetTimerfd(timerfd_, timers_.begin()->first);
            }
        });
    }
