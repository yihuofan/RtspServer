#ifndef RTSPSERVER_THREAD_H
#define RTSPSERVER_THREAD_H

#include <pthread.h>

class Thread
{
public:
    virtual ~Thread();

    bool start(void *arg);
    bool detach();
    bool join();

protected:
    Thread();

    virtual void run(void *arg) = 0;

private:
    static void *threadRun(void *);

private:
    void *mArg;
    bool mIsStart;
    bool mIsDetach;
    pthread_t mThreadId;
};

#endif 