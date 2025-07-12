#include <stdio.h>

#include "Event.h"
#include "Log.h"

/*
TriggerEvent表示一个可以手动调用handleEvent()触发的一次性事件
TimerEvent表示定时器事件，时间到达后自动触发
IOEvent表示 I/O 事件（如 socket 可读/可写）I/O 状态变化时触发
*/

TriggerEvent* TriggerEvent::createNew(void* arg){
    return new TriggerEvent(arg);
}

TriggerEvent* TriggerEvent::createNew(){
    return new TriggerEvent(NULL);
}

TriggerEvent::TriggerEvent(void* arg) : mArg(arg), mTriggerCallback(NULL){
    LOGI("TriggerEvent()");
}
TriggerEvent::~TriggerEvent() {
    LOGI("~TriggerEvent()");

}

void TriggerEvent::handleEvent(){
    if(mTriggerCallback)
        mTriggerCallback(mArg);
}

TimerEvent* TimerEvent::createNew(void* arg){
    return new TimerEvent(arg);
}

TimerEvent* TimerEvent::createNew(){
    return new TimerEvent(NULL);
}

TimerEvent::TimerEvent(void* arg) : mArg(arg), 
mTimeoutCallback(NULL),
mIsStop(false){
    LOGI("TimerEvent()");
}
TimerEvent::~TimerEvent() {
    LOGI("~TimerEvent()");
}

bool TimerEvent::handleEvent()
{
    if (mIsStop) {
        return mIsStop;
    }

    if(mTimeoutCallback)
        mTimeoutCallback(mArg);

    return mIsStop;
}
void TimerEvent::stop() {
    mIsStop = true;
}
IOEvent* IOEvent::createNew(int fd, void* arg){
    if(fd < 0)
        return NULL;

    return new IOEvent(fd, arg);
}

IOEvent* IOEvent::createNew(int fd){
    if(fd < 0)
        return NULL;
   
    return new IOEvent(fd, NULL);
}

IOEvent::IOEvent(int fd, void* arg) :
    mFd(fd),
    mArg(arg),
    mEvent(EVENT_NONE),//当前关注的事件类型
    mREvent(EVENT_NONE),//当前就绪的事件类型
    mReadCallback(NULL),
    mWriteCallback(NULL),
    mErrorCallback(NULL){

    LOGI("IOEvent() fd=%d",mFd);
}

IOEvent::~IOEvent() {
    LOGI("~IOEvent() fd=%d", mFd);
}

void IOEvent::handleEvent()
{
    if (mReadCallback && (mREvent & EVENT_READ))
    {
        mReadCallback(mArg);
    }

    if (mWriteCallback && (mREvent & EVENT_WRITE))//未实现
    {
        mWriteCallback(mArg);
    }
    
    if (mErrorCallback && (mREvent & EVENT_ERROR))//未实现
    {
        mErrorCallback(mArg);
    }
};
