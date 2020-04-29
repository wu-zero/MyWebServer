/// \file EventLoopThreadPool.h
///
/// EventLoop线程池, 创建多个EventLoop线程, 返回可用EventLoop
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/23.


#ifndef MYWEBSERVER_EVENTLOOPTHREADPOOL_H
#define MYWEBSERVER_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
private:
public:
    explicit EventLoopThreadPool(EventLoop *baseLoop, int numThreads = 0);
    ~EventLoopThreadPool();
    /// 线程池创建
    void start();
    /// 返回一个可用线程对应的Loop
    EventLoop *getNextLoop();
private:
    EventLoop *mBaseLoop;
    bool mIsStarted;
    int mNumThread;
    int mNext;
    std::vector<std::shared_ptr<EventLoopThread>> mThreadVec;
    std::vector<EventLoop *> mLoopVec;
};


#endif //MYWEBSERVER_EVENTLOOPTHREADPOOL_H
