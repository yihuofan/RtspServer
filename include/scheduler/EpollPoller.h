#ifndef RTSPSERVER_EPOLLPOLLER_H
#define RTSPSERVER_EPOLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <unistd.h>

class EpollPoller : public Poller
{
public:
    EpollPoller();
    virtual ~EpollPoller();

    static EpollPoller *createNew();

    virtual bool addIOEvent(IOEvent *event);
    virtual bool updateIOEvent(IOEvent *event);
    virtual bool removeIOEvent(IOEvent *event);
    virtual void handleEvent();

private:
    static const int kMaxEvents = 1024;

    int mEpollFd;
    struct epoll_event mEvents[kMaxEvents];
    std::map<int, IOEvent *> mIOEventMap; // fd -> IOEvent 映射

    uint32_t eventToEpoll(int event);
    void epollToEvent(uint32_t epollEvents, IOEvent *ioEvent);
};

#endif