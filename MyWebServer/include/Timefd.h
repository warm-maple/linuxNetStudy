#pragma once

#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include "Timestamp.h"

inline int createTimefd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        perror("Failed to create timerfd");
    }
    return timerfd;
}

inline struct itimerspec howMuchTime(Timestamp expiration) {
    int64_t microseconds = expiration.getMicroSeconds() - Timestamp::now().getMicroSeconds();
    if (microseconds < 100) microseconds = 100;
    struct itimerspec value;
    memset(&value, 0, sizeof(value));
    value.it_value.tv_sec = static_cast<time_t>(microseconds / Timestamp::perSecond);
    value.it_value.tv_nsec = static_cast<long>((microseconds % Timestamp::perSecond) * 1000);
    return value;
}

inline void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newvalue = howMuchTime(expiration);
    ::timerfd_settime(timerfd, 0, &newvalue, NULL);
}