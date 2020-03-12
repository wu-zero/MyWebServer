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
#include "iostream"
Channel::Channel(int epollFd)
        : mEpollFd(epollFd),
          mSocketFd(-1),
          mEvents(0),
          mRevents(0)
{
}

Channel::~Channel()
{
}


void Channel::setRevents(__uint32_t revents)
{
    mRevents = revents;
}

void Channel::handleEvent()
{
    std::cout << "mEvents " << mEvents << std::endl;
    std::cout << "mRevents " << mRevents <<std::endl;
    if (mRevents & EPOLLIN)
    {
        mReadHandler();
    }
    if(mRevents & EPOLLOUT)
    {
        mWriteHandler();
    }
}
void Channel::setEvents(__uint32_t event)
{
    mEvents = event;
    update();
}

void Channel::enableReading()
{
    mEvents |= (EPOLLIN | EPOLLPRI);
    update();
}

void Channel::enableWriting()
{
    mEvents |= EPOLLOUT;
    update();
}

void Channel::update()
{
    struct epoll_event event;
    event.data.ptr = this;
    event.events = mEvents;
    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mSocketFd, &event);
}

int Channel::getSocketFd()
{
    return mSocketFd;
}

void Channel::setSocketFd(int socketFd)
{
    mSocketFd = socketFd;
}

void Channel::setReadHandler(const Callback &cb)
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




