/// \file EventLoopThread.cpp
///
/// EventLoop线程，创建一个线程，在该线程里运行Loop
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/23.


#include "EventLoopThread.h"

#include <iostream>

#include "EventLoop.h"


EventLoopThread::EventLoopThread()
        : mLoop(nullptr),
          mIsRunning(false),
          mThread()
{

}

EventLoopThread::~EventLoopThread() {
    mIsRunning = false;
    if(mLoop != nullptr){
        mLoop->quit();
    }
    mThread.join();
}

EventLoop *EventLoopThread::startLoop() {
    mIsRunning = true;
    mThread = std::thread(std::bind(&EventLoopThread::threadFunc,this));
    {
        std::unique_lock<std::mutex> lock(mMutex);
        // 等待threadFunc获取mLoop
        while(mLoop == nullptr){
            mCond.wait(lock);
        }
    }
    return mLoop;
}

void EventLoopThread::threadFunc() {
    std::cout << "EventLoopThread::threadFunc id: "<<mThread.get_id() << std::endl;
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mLoop = &loop;
        mCond.notify_one();
    }
    loop.loop();
    mLoop = nullptr;
}
