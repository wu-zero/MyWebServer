/// \file EventLoopThreadPool.cpp
///
/// EventLoop线程池, 创建多个EventLoop线程, 返回可用EventLoop
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/23.


#include "EventLoopThreadPool.h"

#include <iostream>
#include <assert.h>

#include "EventLoop.h"
#include "EventLoopThread.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
        : mBaseLoop(baseLoop),
          mIsStarted(false),
          mNumThread(numThreads),
          mNext(0)
{
    std::cout << "EventLoopThreadPool::EventLoopThreadPool" <<std::endl;
}

EventLoopThreadPool::~EventLoopThreadPool() {
    std::cout << "EventLoopThreadPool::~EventLoopThreadPool" <<std::endl;
}


void EventLoopThreadPool::start() {
    assert(mBaseLoop->isInLoopThread());
    mIsStarted = true;
    for(int i=0; i<mNumThread; ++i){
        std::shared_ptr<EventLoopThread> sPtrEventLoopThread(new EventLoopThread());
        mThreadVec.push_back(sPtrEventLoopThread);
        mLoopVec.push_back(sPtrEventLoopThread->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    assert(mBaseLoop->isInLoopThread());
    assert(mIsStarted);
    EventLoop *loop = mBaseLoop;
    if (!mLoopVec.empty()) {
        loop = mLoopVec[mNext];
        mNext = (mNext + 1) % mNumThread;
    }
    return loop;
}
