#include "EpollPoller.h"
#include "Event.h"
#include "Log.h"

EpollPoller::EpollPoller() : mEpollFd(-1)
{
    mEpollFd = epoll_create1(EPOLL_CLOEXEC); // 创建 epoll 实例，设置该描述符的 FD_CLOEXEC 标志。避免描述符保留并传递给新程序。
    if (mEpollFd < 0)
    {
        LOGE("EpollPoller::EpollPoller() epoll_create1 failed");
    }
    else
    {
        LOGI("EpollPoller::EpollPoller() epoll_fd=%d", mEpollFd);
    }
}

EpollPoller::~EpollPoller()
{
    if (mEpollFd >= 0)
    {
        close(mEpollFd);
        LOGI("EpollPoller::~EpollPoller() epoll_fd=%d closed", mEpollFd);
    }
}

EpollPoller *EpollPoller::createNew()
{
    return new EpollPoller();
}

bool EpollPoller::addIOEvent(IOEvent *event)
{
    if (!event || mEpollFd < 0)
    {
        return false;
    }

    int fd = event->getFd();
    if (mIOEventMap.find(fd) != mIOEventMap.end())
    {
        LOGE("EpollPoller::addIOEvent() fd=%d already exists", fd);
        return false;
    }

    struct epoll_event epollEvent;
    epollEvent.events = eventToEpoll(event->getEvent());
    epollEvent.data.fd = fd;

    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &epollEvent) < 0)
    {
        LOGE("EpollPoller::addIOEvent() epoll_ctl ADD failed, fd=%d", fd);
        return false;
    }

    mIOEventMap[fd] = event;
    LOGI("EpollPoller::addIOEvent() fd=%d added", fd);
    return true;
}

bool EpollPoller::updateIOEvent(IOEvent *event)
{
    if (!event || mEpollFd < 0)
    {
        return false;
    }

    int fd = event->getFd();
    if (mIOEventMap.find(fd) == mIOEventMap.end())
    {
        LOGE("EpollPoller::updateIOEvent() fd=%d not found", fd);
        return false;
    }

    struct epoll_event epollEvent;
    epollEvent.events = eventToEpoll(event->getEvent());
    epollEvent.data.fd = fd;

    if (epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &epollEvent) < 0)
    {
        LOGE("EpollPoller::updateIOEvent() epoll_ctl MOD failed, fd=%d", fd);
        return false;
    }

    LOGI("EpollPoller::updateIOEvent() fd=%d updated", fd);
    return true;
}

bool EpollPoller::removeIOEvent(IOEvent *event)
{
    if (!event || mEpollFd < 0)
    {
        return false;
    }

    int fd = event->getFd();
    auto it = mIOEventMap.find(fd);
    if (it == mIOEventMap.end())
    {
        LOGE("EpollPoller::removeIOEvent() fd=%d not found", fd);
        return false;
    }

    if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr) < 0)
    {
        LOGE("EpollPoller::removeIOEvent() epoll_ctl DEL failed, fd=%d", fd);
        return false;
    }

    mIOEventMap.erase(it);
    LOGI("EpollPoller::removeIOEvent() fd=%d removed", fd);
    return true;
}

void EpollPoller::handleEvent()
{
    if (mEpollFd < 0)
    {
        return;
    }

    int numEvents = epoll_wait(mEpollFd, mEvents, kMaxEvents, -1);
    if (numEvents < 0)
    {
        LOGE("EpollPoller::handleEvent() epoll_wait failed");
        return;
    }

    for (int i = 0; i < numEvents; ++i)
    {
        int fd = mEvents[i].data.fd;
        auto it = mIOEventMap.find(fd);
        if (it != mIOEventMap.end())
        {
            IOEvent *ioEvent = it->second;
            epollToEvent(mEvents[i].events, ioEvent);
            ioEvent->handleEvent();
        }
    }
}

uint32_t EpollPoller::eventToEpoll(int event)// 将IOEvent事件转换为epoll事件
{
    uint32_t epollEvents = 0;

    if (event & IOEvent::EVENT_READ)
    {
        epollEvents |= EPOLLIN;
    }
    if (event & IOEvent::EVENT_WRITE)
    {
        epollEvents |= EPOLLOUT;
    }
    if (event & IOEvent::EVENT_ERROR)
    {
        epollEvents |= EPOLLERR;
    }

    // 默认使用边缘触发模式
    epollEvents |= EPOLLET;

    return epollEvents;
}

void EpollPoller::epollToEvent(uint32_t epollEvents, IOEvent *ioEvent) // 将epoll事件转换为IOEvent事件
{
    int events = IOEvent::EVENT_NONE;

    if (epollEvents & EPOLLIN)
    {
        events |= IOEvent::EVENT_READ;
    }
    if (epollEvents & EPOLLOUT)
    {
        events |= IOEvent::EVENT_WRITE;
    }
    if (epollEvents & (EPOLLERR | EPOLLHUP))
    {
        events |= IOEvent::EVENT_ERROR;
    }

    ioEvent->setREvent(events);
}