// EventLoop.h - 支持多线程的事件循环
#pragma once

#include "Channel.h"
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include "TimerQueue.h"

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 事件循环
    void loop();
    void quit();

    // 获取 epoll fd
    int getFd() const { return epfd_; }

    // 【核心】跨线程调度函数
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    // 唤醒阻塞的 epoll_wait
    void wakeup();

    // 定时器接口：在 EventLoop 上添加定时器（转发到内部 TimerQueue）
    TimerQueue::TimerId addTimer(TimerQueue::TimerCallback cb, Timestamp when, double interval);
    void cancelTimer(TimerQueue::TimerId id);

    // 判断当前是否在 EventLoop 所属线程
    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

    // Channel 操作
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    void handleWakeup();
    void doPendingFunctors();

    int epfd_;
    std::vector<struct epoll_event> events_;
    
    std::atomic<bool> quit_;
    std::thread::id threadId_;  // 创建 EventLoop 的线程ID

    // 用于唤醒 epoll_wait
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    // 定时器队列，负责管理定时器（集成到 EventLoop）
    std::unique_ptr<TimerQueue> timerQueue_;

    // 待执行的回调函数队列
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
    std::atomic<bool> callingPendingFunctors_;
};