#include "Thread.h"
#include <pthread.h>
#include <unistd.h>

Thread::Thread() : mArg(nullptr),
                   mIsStart(false),
                   mIsDetach(false)
{
    mThreadId = 0;
}
Thread::~Thread()
{
    if (mIsStart == true && mIsDetach == false)
        detach();
}

bool Thread::start(void *arg)
{
    mArg = arg;
    if (pthread_create(&mThreadId, NULL, threadRun, this) != 0)
    {
        return false;
    }
    mIsStart = true;
    return true;
}

bool Thread::detach()
{
    if (mIsStart != true)
        return false;

    if (mIsDetach == true)
        return true;

    if (pthread_detach(mThreadId) != 0)
        return false;

    mIsDetach = true;
    return true;
}

bool Thread::join()
{
    if (mIsStart != true || mIsDetach == true)
        return false;

    if (pthread_join(mThreadId, NULL) != 0)
        return false;

    return true;
}

void *Thread::threadRun(void *arg)
{
    Thread *thread = (Thread *)arg;
    thread->run(thread->mArg);
    return nullptr;
}