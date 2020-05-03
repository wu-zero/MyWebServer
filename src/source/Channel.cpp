/// \file Channel.cpp
///
/// 每个 Channel 对象负责一个文件描述符，
/// 需要设置要监听的事件类型以及对应的回调函数，
/// 执行回调时，先指明监听到的事件类型，再执行回调。
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.

#include "Channel.h"

#include "sys/epoll.h"
#include <iostream>
#include <utility>

#include "EventLoop.h"


Channel::Channel(EventLoop *loop, int fileDescriptor)
        : mLoop(loop),
          mFd(fileDescriptor),
          mEvents(0),
          mRevents(0),
          mReadHandler(nullptr),
          mWriteHandler(nullptr),
          mCloseHandler(nullptr),
          mStatus(kChannelStatuesNew)
{
}

Channel::~Channel()
{
}

void Channel::remove()
{
    mLoop->removeChannel(this);
}

int Channel::getFd()
{
    return mFd;
}

void Channel::setFd(int fileDescriptor)
{
    mFd = fileDescriptor;
}

// 设置监听事件
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

// 获得监听事件
__uint32_t Channel::getEvents() const
{
    return mEvents;
}

bool Channel::isWriting()
{
    return mEvents & EPOLLOUT;
}

bool Channel::isReading()
{
    return mEvents & EPOLLIN;
}

//设置活动事件类型
void Channel::setRevents(__uint32_t revents)
{
    mRevents = revents;
}

//处理回调
void Channel::handleEvent()
{
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
    if (mRevents & (EPOLLIN| EPOLLPRI ))
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
            std::cout << "mWriteHandler" << mFd<< std::endl;
            mWriteHandler();
        }
    }
}

//设置回调函数
void Channel::setReadHandler(Channel::Callback cb)
{
    mReadHandler = std::move(cb);
}

void Channel::setWriteHandler(Channel::Callback cb)
{
    mWriteHandler = std::move(cb);
}

void Channel::setCloseHandler(Channel::Callback cb)
{
    mCloseHandler = std::move(cb);
}

// Channel负责的fd在epoll中的现状态
void Channel::setStatus(ChannelStatus status)
{
    mStatus = status;
}

Channel::ChannelStatus Channel::getStatus()
{
    return mStatus;
}

//epoll里更新需要监听的事件类型
void Channel::update()
{
    mLoop->updateChannel(this);
}



