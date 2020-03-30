/// \file EventLoop.cpp
///
/// EventLoop的主要功能是运行事件循环, 每个EventLoop负责一个Epoll, 通过Eventfd实现异步处理事件的能力
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.

#include "EventLoop.h"

#include <iostream>
#include <sys/eventfd.h>
#include "zconf.h"
#include <unistd.h>

#include "Epoll.h"
#include "Channel.h"

EventLoop::EventLoop()
        : mIsQuit(false),
          mEpoll(new Epoll()),
          mWakeUpEventFd(createEventFd()),
          mWakeUpChannel(new Channel(this, mWakeUpEventFd)),
          mPendingFunctors()
{
    mWakeUpChannel->setReadHandler(std::bind(&EventLoop::handleRead, this));
    mWakeUpChannel->enableReading();
}

EventLoop::~EventLoop()
{
    close(mWakeUpEventFd);
}

void EventLoop::loop()
{
    mIsQuit = false;
    while (!mIsQuit)
    {
        std::vector<Channel *> retChannels;
        mEpoll->poll(&retChannels);
        std::vector<Channel *>::iterator it;
        for (it = retChannels.begin(); it != retChannels.end(); ++it)
        {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    mEpoll->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel){
    mEpoll->removeChannel(channel);
}

void EventLoop::queueLoop(const Functor &functor)
{
    mPendingFunctors.push_back(functor);
    wakeup();
}

// 往 mWakeUpEventFd 写入一个字节的数据，唤醒 IO 线程
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(mWakeUpEventFd, &one, sizeof one);
    if (n != sizeof(one))
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(mWakeUpEventFd, &one, sizeof one);
    if (n != sizeof(one))
    {
        std::cout << "EventEventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> tempFunctors;
    tempFunctors.swap(mPendingFunctors);
    for (const auto &functor: tempFunctors)
    {
        functor();
    }
}


int EventLoop::createEventFd()
{

    int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0)
    {
        std::cout << "Failed in eventfd" << std::endl;
    }
    return eventFd;
}
