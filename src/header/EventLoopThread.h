/// \file EventLoopThread.h
///
/// EventLoop线程，创建一个线程，在该线程里运行Loop
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/23.

#include <thread>
#include <mutex>
#include <condition_variable>


#ifndef MYWEBSERVER_EVENTLOOPTHREAD_H
#define MYWEBSERVER_EVENTLOOPTHREAD_H

class EventLoop;

class EventLoopThread {
public:

private:
    EventLoop *mLoop;
    bool mIsRunning;
    std::thread mThread;
    std::mutex mMutex; // 保证mLoop指针的线程安全
    std::condition_variable mCond; // 保证mLoop指针的线程安全
public:
    EventLoopThread();
    ~EventLoopThread();
    /// 返回创建的EventLoop指针
    EventLoop *startLoop();
private:
    /// 在新建的线程中执行，在新建的线程中创建EventLoop
    void threadFunc();
};


#endif //MYWEBSERVER_EVENTLOOPTHREAD_H
