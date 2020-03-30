/// \file Channel.cpp
///
/// 每个 Channel 对象负责一个socket文件描述符，
/// 需要设置要监听的事件类型以及对应的回调函数，
/// 执行回调时，先指明监听到的事件类型，再执行回调。
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.

#include "Channel.h"

#include "sys/epoll.h"
#include <iostream>

#include "EventLoop.h"


Channel::Channel(EventLoop *loop, int socketFd)
        : mLoop(loop),
          mSocketFd(socketFd),
          mEvents(0),
          mRevents(0),
          mReadHandler(nullptr),
          mWriteHandler(nullptr),
          mCloseHandler(nullptr),
          mStatus(-1)
{
}

Channel::~Channel()
{
}
void Channel::remove()
{
    mLoop->removeChannel(this);
}

int Channel::getSocketFd()
{
    return mSocketFd;
}

void Channel::setSocketFd(int socketFd)
{
    mSocketFd = socketFd;
}

void Channel::setEvents(__uint32_t event)
{
    mEvents = event;
    update();
}

__uint32_t Channel::getEvents() const
{
    return mEvents;
}

void Channel::setRevents(__uint32_t revents)
{
    mRevents = revents;
}

void Channel::handleEvent()
{
//    std::cout << "mSocketFd: " << mSocketFd << " "
//              << "mEvents: " << mEvents << " "
//              << "mRevents: " << mRevents << std::endl;
    if (mRevents != 1){
        std::cout << mRevents<<std::endl;
    }
    // 处理断开事件
    if ((mRevents & EPOLLHUP) && !(mRevents & EPOLLIN))
    {
        std::cout << "close" << std::endl;
        if (mCloseHandler)
        {
            mCloseHandler();
        }
    }
    // 处理可读事件
    if (mRevents & EPOLLIN)
    {
        if (mReadHandler)
        {
            mReadHandler();
        }
    }
    // 处理可写事件
    if (mRevents & EPOLLOUT)
    {
        if (mWriteHandler)
        {
            mWriteHandler();
        }
    }
}


void Channel::enableReading()
{
    mEvents |= (EPOLLIN | EPOLLPRI);
    update();
}

void Channel::disableReading()
{
    mEvents &= ~(EPOLLIN | EPOLLPRI);
    update();
}

void Channel::enableWriting()
{
    mEvents |= EPOLLOUT;
    update();
}

void Channel::disableWriting()
{
    mEvents &= ~EPOLLOUT;
    update();
}

void Channel::disableAll()
{
    mEvents = 0;
    update();
}

void Channel::update()
{
    mLoop->updateChannel(this);
}


void Channel::setReadHandler(const Channel::Callback &cb)
{
    mReadHandler = cb;
}

void Channel::setWriteHandler(const Channel::Callback &cb)
{
    mWriteHandler = cb;
}

void Channel::setCloseHandler(const Channel::Callback &cb)
{
    mCloseHandler = cb;
}

bool Channel::isWriting()
{
    return mEvents & EPOLLOUT;
}

int Channel::getStatus()
{
    return mStatus;
}

void Channel::setStatus(int status)
{
    mStatus = status;
}



