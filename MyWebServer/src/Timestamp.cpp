#include "Timestamp.h"
#include <sys/time.h>
#include <cstdio>

Timestamp Timestamp::now() {
   struct timeval t;
   gettimeofday(&t, NULL);
   int64_t second = t.tv_sec;
   return Timestamp(second * Timestamp::perSecond + t.tv_usec);
}
std::string Timestamp::toString() const {
   char buf[128]={0};
   time_t seconds=static_cast<time_t>(microSeconds_/perSecond);
   int64_t microSeconds=microSeconds_%perSecond;
   struct tm tm_time;
   gmtime_r(&seconds,&tm_time);
   snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06ld",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microSeconds);
    return buf;
}