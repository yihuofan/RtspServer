#ifndef RTSPSERVER_EVENTSCHEDULER_H
#define RTSPSERVER_EVENTSCHEDULER_H

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include "Timer.h"
#include "Event.h"
class Poller;

/*
用于调度和管理事件的类，支持触发事件、定时事件和IO事件。
*/

class EventScheduler
{
public:
    
    static EventScheduler* createNew();

    explicit EventScheduler();
    virtual ~EventScheduler();
public:
    bool addTriggerEvent(TriggerEvent* event);
    Timer::TimerId addTimedEventRunAfater(TimerEvent* event, Timer::TimeInterval delay);
    Timer::TimerId addTimedEventRunAt(TimerEvent* event, Timer::Timestamp when);
    Timer::TimerId addTimedEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
    bool removeTimedEvent(Timer::TimerId timerId);
    bool addIOEvent(IOEvent* event);
    bool updateIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);

    void loop();
    Poller* poller();

private:
    void handleTriggerEvents();

private:
    bool mQuit;
    Poller* mPoller;
    TimerManager* mTimerManager;
    std::vector<TriggerEvent*> mTriggerEvents;

    std::mutex mMtx;
};

#endif 