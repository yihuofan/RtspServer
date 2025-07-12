#include "EventScheduler.h"
#include "SocketsOps.h"
#include "EpollPoller.h"
#include "Log.h"

#include <sys/eventfd.h>

EventScheduler *EventScheduler::createNew()
{
    
    return new EventScheduler();
}

EventScheduler::EventScheduler() : mQuit(false)
{
    mPoller = EpollPoller::createNew();
    mTimerManager = TimerManager::createNew(this);
}

EventScheduler::~EventScheduler()
{
    delete mTimerManager;
    delete mPoller;
}

bool EventScheduler::addTriggerEvent(TriggerEvent *event)
{
    mTriggerEvents.push_back(event);
    return true;
}

Timer::TimerId EventScheduler::addTimedEventRunAfater(TimerEvent *event, Timer::TimeInterval delay)
{
    Timer::Timestamp timestamp = Timer::getCurTime();
    timestamp += delay;
    return mTimerManager->addTimer(event, timestamp, 0);
}

Timer::TimerId EventScheduler::addTimedEventRunAt(TimerEvent *event, Timer::Timestamp when)
{
    return mTimerManager->addTimer(event, when, 0);
}

Timer::TimerId EventScheduler::addTimedEventRunEvery(TimerEvent *event, Timer::TimeInterval interval) // 周期性地执行任务
{
    Timer::Timestamp timestamp = Timer::getCurTime();// 获取当前时间戳
    timestamp += interval;// 设置下次执行时间
    return mTimerManager->addTimer(event, timestamp, interval);// 添加定时器
}

bool EventScheduler::removeTimedEvent(Timer::TimerId timerId)
{
    return mTimerManager->removeTimer(timerId);
}

bool EventScheduler::addIOEvent(IOEvent *event)
{
    return mPoller->addIOEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent *event)
{
    return mPoller->updateIOEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent *event)
{
    return mPoller->removeIOEvent(event);
}

void EventScheduler::loop()
{
    while (!mQuit)
    {
        handleTriggerEvents();
        mPoller->handleEvent();
    }
}

void EventScheduler::handleTriggerEvents()
{
    if (!mTriggerEvents.empty())
    {
        for (std::vector<TriggerEvent *>::iterator it = mTriggerEvents.begin();
             it != mTriggerEvents.end(); ++it)
        {
            (*it)->handleEvent();
        }
        mTriggerEvents.clear();
    }
}

Poller *EventScheduler::poller()
{
    return mPoller;
}
