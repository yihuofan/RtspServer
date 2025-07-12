#include "Timer.h"
#include <sys/timerfd.h>
#include <time.h>
#include <chrono>
#include "Event.h"
#include "EventScheduler.h"
#include "Poller.h"
#include "Log.h"

static bool timerFdSetTime(int fd, Timer::Timestamp when, Timer::TimeInterval period)
{
    struct itimerspec newVal;

    newVal.it_value.tv_sec = when / 1000;                  // ms -> s
    newVal.it_value.tv_nsec = (when % 1000) * 1000 * 1000; // ms -> ns
    newVal.it_interval.tv_sec = period / 1000;
    newVal.it_interval.tv_nsec = (period % 1000) * 1000 * 1000;

    int oldValue = timerfd_settime(fd, TFD_TIMER_ABSTIME, &newVal, NULL);
    if (oldValue < 0)
    {
        LOGE("timerfd_settime error");
        return false;
    }
    return true;
}

Timer::Timer(TimerEvent *event, Timestamp timestamp, TimeInterval timeInterval, TimerId timerId)
    : mTimerEvent(event),
      mTimestamp(timestamp),
      mTimeInterval(timeInterval),
      mTimerId(timerId)
{
    mRepeat = (timeInterval > 0);
}

Timer::~Timer()
{
}

// 获取系统从启动到目前的毫秒数（适合用于相对时间判断）
Timer::Timestamp Timer::getCurTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

// 获取当前系统时间戳（13位毫秒级，适合用于日志或绝对时间）
Timer::Timestamp Timer::getCurTimestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

bool Timer::handleEvent()
{
    if (!mTimerEvent)
    {
        return false;
    }
    return mTimerEvent->handleEvent();
}

TimerManager *TimerManager::createNew(EventScheduler *scheduler)// 创建定时器管理器
{
    if (!scheduler)
        return nullptr;
    return new TimerManager(scheduler);
}

TimerManager::TimerManager(EventScheduler *scheduler)
    : mPoller(scheduler->poller()),
      mLastTimerId(0)
{
    mTimerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (mTimerFd < 0)
    {
        LOGE("Failed to create timerfd");
        return;
    }

    LOGI("timerfd=%d", mTimerFd);

    mTimerIOEvent = IOEvent::createNew(mTimerFd, this);
    mTimerIOEvent->setReadCallback(readCallback);
    mTimerIOEvent->enableReadHandling();
    modifyTimeout();
    mPoller->addIOEvent(mTimerIOEvent);
}

TimerManager::~TimerManager()
{
    mPoller->removeIOEvent(mTimerIOEvent);
    delete mTimerIOEvent;
}

Timer::TimerId TimerManager::addTimer(TimerEvent *event, 
                                      Timer::Timestamp timestamp,//触发时间
                                      Timer::TimeInterval timeInterval)//执行间隔
{
    ++mLastTimerId;
    Timer timer(event, timestamp, timeInterval, mLastTimerId);

    mTimers.insert(std::make_pair(mLastTimerId, timer));
    mEvents.insert(std::make_pair(timestamp, timer));

    modifyTimeout();

    return mLastTimerId;
}

bool TimerManager::removeTimer(Timer::TimerId timerId)
{
    auto it = mTimers.find(timerId);
    if (it != mTimers.end())
    {
        mTimers.erase(it);
        // 这里没有删除 mEvents 中的条目，可使用更高效的数据结构或增加同步逻辑
        modifyTimeout();
    }
    return true;
}

void TimerManager::modifyTimeout()
{
    auto it = mEvents.begin();
    if (it != mEvents.end())
    {
        const Timer &timer = it->second;
        timerFdSetTime(mTimerFd, timer.mTimestamp, timer.mTimeInterval);
    }
    else
    {
        timerFdSetTime(mTimerFd, 0, 0); // 关闭定时器
    }
}

void TimerManager::readCallback(void *arg)
{
    TimerManager *timerManager = static_cast<TimerManager *>(arg);
    timerManager->handleRead();
}

void TimerManager::handleRead()
{
    Timer::Timestamp now = Timer::getCurTime();

    if (mEvents.empty())
    {
        return;
    }

    auto it = mEvents.begin();
    Timer timer = it->second;

    if (now >= timer.mTimestamp)
    {
        mEvents.erase(it);

        bool stop = timer.handleEvent();

        if (timer.mRepeat)
        {
            if (stop)
            {
                mTimers.erase(timer.mTimerId);
            }
            else
            {
                timer.mTimestamp = now + timer.mTimeInterval;
                mEvents.insert(std::make_pair(timer.mTimestamp, timer));
            }
        }
        else
        {
            mTimers.erase(timer.mTimerId);
        }

        modifyTimeout();
    }
}