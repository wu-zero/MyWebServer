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
#include <assert.h>

#include "Epoll.h"
#include "Channel.h"
#include "Timer.h"

EventLoop::EventLoop()
        : mIsLooping(false),
          mIsQuit(false),
          mSPtrEpoll(new Epoll()),
          mWakeUpEventFd(createEventFd()),
          mSPtrWakeUpChannel(new Channel(this, mWakeUpEventFd)),
          mPendingFunctors(),
          mSPtrTimerManager(new TimerManager(this)),
          mTid(std::this_thread::get_id())
{
    mSPtrWakeUpChannel->setReadHandler(std::bind(&EventLoop::handleRead, this));
    mSPtrWakeUpChannel->enableReading();
}

EventLoop::~EventLoop()
{
    mSPtrWakeUpChannel->disableAll(); // 关闭监听
    mSPtrWakeUpChannel->remove(); // 从Epoll中删除
    close(mWakeUpEventFd);
}

void EventLoop::quit() {
    std::cout << "EventLoop::quit()" << std::endl;
    mIsQuit = true;
    if(!isInLoopThread()){
        wakeup();
    }
}

void EventLoop::loop()
{
    assert(!mIsLooping);
    assert(isInLoopThread());
    mIsLooping = true;
    std::vector<Channel *> retChannels;
    while (!mIsQuit)
    {
        retChannels.clear();
        mSPtrEpoll->poll(retChannels);
        std::vector<Channel *>::iterator it;
        for (it = retChannels.begin(); it != retChannels.end(); ++it)
        {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }
    std::cout << "EventLoop::loop() stop" <<std::endl;
    mIsLooping = false;
}

void EventLoop::updateChannel(Channel *channel)
{
    mSPtrEpoll->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel){
    mSPtrEpoll->removeChannel(channel);
}

void EventLoop::runInLoop(EventLoop::Functor &&functor) {
    if(isInLoopThread())
        functor();
    else{
        queueLoop(std::move(functor));
    }
}

std::thread::id EventLoop::getThreadId() {
    return mTid;
}

bool EventLoop::isInLoopThread() const {
    return mTid == std::this_thread::get_id();
}

void EventLoop::assertInLoopThread() {
    assert(isInLoopThread());
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

// 往 mWakeUpEventFd 写入一个字节的数据，唤醒线程
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(mWakeUpEventFd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::queueLoop(Functor &&functor)
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mPendingFunctors.emplace_back(std::move(functor));
    }
    if(!isInLoopThread())
        wakeup();
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> tempFunctors;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        tempFunctors.swap(mPendingFunctors);
    }
    for (const auto &functor: tempFunctors)
    {
        functor();
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












