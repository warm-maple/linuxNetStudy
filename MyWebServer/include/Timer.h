#pragma once

#include "Timestamp.h"
#include <functional>

class Timer {
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) {}

    void run() const { callback_(); }
    Timestamp getexpiration() const { return expiration_; }
    bool isRepeat() const { return repeat_; }
    void restart(Timestamp now) {
        if (repeat_)
            expiration_ = addTime(now, interval_);
    }

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
};