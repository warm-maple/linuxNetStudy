#include <iostream>
#include <unistd.h>
#include "../include/EventLoop.h"
#include "../include/TimerQueue.h"
#include "../include/Timestamp.h"

void print(const char* msg)
{
    std::cout << Timestamp::now().toString() << " " << msg << std::endl;
}

void cancel(int* count, EventLoop* loop)
{
    (*count)++;
    if (*count >= 3)
    {
        print("Cancelling loop");
        loop->quit();
    }
}

int main()
{
    print("main");
    EventLoop loop;
    TimerQueue timerQueue(&loop);

    print("Adding timers");
    
    // One shot timer
    timerQueue.addTimer([](){ print("One shot timer called"); }, addTime(Timestamp::now(), 1.0), 0.0);
    
    // Repeating timer
    timerQueue.addTimer([](){ print("Repeating timer called"); }, addTime(Timestamp::now(), 0.5), 0.5);

    // Timer to quit loop
    int count = 0;
    // We don't have a way to cancel a specific timer easily from outside in this simplified implementation
    // So we just rely on a separate timer or the repeating timer to quit.
    // Let's add another repeating timer to check count and quit.
    timerQueue.addTimer(std::bind(cancel, &count, &loop), addTime(Timestamp::now(), 1.0), 1.0);

    loop.loop();
    print("main loop exited");
    return 0;
}
