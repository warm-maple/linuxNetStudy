#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H
#include <vector>
#include <set>
#include <memory>
#include <functional>
#include "Timestamp.h"
#include "Channel.h"
class EventLoop;
class Timer;

class TimerQueue {
public:
    using TimerCallback = std::function<void()>;
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    // TimerId 对外表示定时器句柄，这里使用 Timer* 作为简单的标识
    using TimerId = Timer*;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();
    // 返回 TimerId，可用于 later 取消定时器
    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    // 取消定时器（线程安全，支持跨线程调用）
    void cancel(TimerId id);
private:
    void handleRead();
    void resetTimerfd(int timerfd, Timestamp expiration);

    EventLoop* loop_;        
    const int timerfd_;       
    Channel timerfdChannel_;  
    TimerList timers_; 
};

#endif