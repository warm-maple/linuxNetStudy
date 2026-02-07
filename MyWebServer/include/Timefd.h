#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include "Timestamp.h"
int createTimefd(){
    int timerfd=::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd==0){}
    return timerfd;
}
struct itimerspec howMuchTime(Timestamp nowtime){
    int64_t microseconds=nowtime.getMicroSeconds()-Timestamp::now().getMicroSeconds();
    if(microseconds<100)microseconds=100;
    struct itimerspec value;
    bzero(&value,sizeof(value));
    value.it_interval.tv_sec=static_cast<time_t>(microseconds / Timestamp::perSecond);
    value.it_value.tv_nsec = static_cast<long>((microseconds % Timestamp::perSecond) * 1000);
    return value;
}
void resetTimerfd(int timerfd,Timestamp expiration){
    struct itimerspec newvalue=howMuchTime(expiration);
    ::timerfd_settime(timerfd,0,&newvalue,NULL);
}