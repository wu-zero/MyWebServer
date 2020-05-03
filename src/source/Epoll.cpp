/// \file Epoll.cpp
///
/// Epoll通过Channel监听Channel负责的文件描述符, 不负责管理Channel
/// 通过updateChannel添加监听fd, 通过removeChannel删除监听fd
/// 只负责与系统相关的epoll部分, 不管理相关对象
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.


#include "Epoll.h"

#include <iostream>
#include <unistd.h>

#include "Channel.h"


Epoll::Epoll()
        : mEpollFd(epoll_create1(EPOLL_CLOEXEC))
{
    if (mEpollFd <= 0)
    {
        std::cout << "epoll_create error, errno:" << mEpollFd << std::endl;
    }
}

Epoll::~Epoll()
{
    close(mEpollFd);
}

void Epoll::poll(ChannelVector &channelVector)
{
    int fds = epoll_wait(mEpollFd, mEvents, kMaxEventListSize, -1);
    if (fds < 0)
    {
        std::cout << "epoll_wait error, errno:" << errno << std::endl;
        return;
    } else
    {
        for (int i = 0; i < fds; i++)
        {
            Channel *ptrChannel = static_cast<Channel *>(mEvents[i].data.ptr);
            ptrChannel->setRevents(mEvents[i].events);
            channelVector.push_back(ptrChannel);
        }
    }
}

void Epoll::updateChannel(Channel *channel)
{
    Channel::ChannelStatus status = channel->getStatus();
    if (status == Channel::kChannelStatuesNew)
    {
        struct epoll_event event{};
        event.data.ptr = channel;
        event.events = channel->getEvents();
        int fd = channel->getFd();

        if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &event) < 0)
        {
            std::cout << "Epoll::updateChannel: EPOLL_CTL_ADD error, errno:" << errno << std::endl;
        }

        channel->setStatus(Channel::kChannelStatuesAdded);
    }
    else
    {
        struct epoll_event event{};
        event.data.ptr = channel;
        event.events = channel->getEvents();
        int fd = channel->getFd();

        if (epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            std::cout << "Epoll::updateChannel: EPOLL_CTL_MOD error, errno:" << errno << std::endl;
        }
    }
}

void Epoll::removeChannel(Channel *channel)
{
    channel->setStatus(Channel::kChannelStatuesDeleted);
    struct epoll_event event{};
    event.data.ptr = channel;
    event.events = channel->getEvents();
    int fd = channel->getFd();

    if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        std::cout << "Epoll::updateChannel: EPOLL_CTL_DEL error, errno:" << errno << std::endl;
    }
}

