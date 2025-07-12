#ifndef RTSPSERVER_TIMER_H
#define RTSPSERVER_TIMER_H

#include <map>
#include <stdint.h>

class EventScheduler;
class Poller;
class TimerEvent;
class IOEvent;

class Timer
{
public:
    typedef uint32_t TimerId;
    typedef int64_t Timestamp;     // 毫秒级时间戳
    typedef uint32_t TimeInterval; // 毫秒间隔

    ~Timer();

    static Timestamp getCurTime();      // 获取系统启动以来的毫秒数
    static Timestamp getCurTimestamp(); // 获取当前时间戳（13位）

private:
    friend class TimerManager;
    Timer(TimerEvent *event, Timestamp timestamp, TimeInterval timeInterval, TimerId timerId);

private:
    bool handleEvent();

    TimerEvent *mTimerEvent;
    Timestamp mTimestamp;
    TimeInterval mTimeInterval;
    TimerId mTimerId;
    bool mRepeat;
};

class TimerManager
{
public:
    static TimerManager *createNew(EventScheduler *scheduler);

    TimerManager(EventScheduler *scheduler);
    ~TimerManager();

    Timer::TimerId addTimer(TimerEvent *event, Timer::Timestamp timestamp,
                            Timer::TimeInterval timeInterval);
    bool removeTimer(Timer::TimerId timerId);

private:
    static void readCallback(void *arg);
    void handleRead();
    void modifyTimeout();

private:
    Poller *mPoller;
    std::map<Timer::TimerId, Timer> mTimers;
    std::multimap<Timer::Timestamp, Timer> mEvents;
    uint32_t mLastTimerId;

    int mTimerFd;
    IOEvent *mTimerIOEvent;
};

#endif 