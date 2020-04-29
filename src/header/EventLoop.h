/// \file EventLoop.h
///
/// EventLoop的主要功能是运行事件循环, 每个EventLoop负责一个Epoll, 通过Eventfd实现异步处理事件的能力
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.


#ifndef MYWEBSERVER_EVENTLOOP_H
#define MYWEBSERVER_EVENTLOOP_H

#include <memory>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>

#include "Timer.h"


class Channel;
class Epoll;
class TimerManager;

// EventLoop 的主要功能是运行事件循环 EventLoop::loop()
class EventLoop
{
public:
    using SPtrEpoll = std::shared_ptr<Epoll>;
    using SPtrTimerManager = std::shared_ptr<TimerManager>;
    using SptrChannel = std::shared_ptr<Channel>;
    using Functor = std::function<void()>; // pending functor 的类型
private:
    bool mIsLooping;
    bool mIsQuit;
    SPtrEpoll mSPtrEpoll; // Epoll对象
    int mWakeUpEventFd; // 用于当前EventLoop的EventFd
    SptrChannel mSPtrWakeUpChannel; // 当前EventFd对应的channel
    std::vector<Functor> mPendingFunctors; // 需要在当前Loop执行的任务，用mMutex维护线程间安全
    SPtrTimerManager mSPtrTimerManager;
    const std::thread::id mTid;
    std::mutex mMutex;

public:
    EventLoop();
    ~EventLoop();
    /// 安全退出Loop
    void quit();
    /// 运行事件循环
    void loop();

    /// Acceptor和Connection的Channel相关
    void updateChannel(Channel *channel); // 添加、更新channel, 主要操作Epoll
    void removeChannel(Channel *channel); // 删除channel, 主要操作Epoll

    /// 在当前Loop里执行任务，应该保证线程安全
    void runInLoop(Functor &&functor);
    void assertInLoopThread();

    /// 获取当前Loop所属threadId
    std::thread::id getThreadId();
    /// 是否处于当前Loop所属的线程
    bool isInLoopThread() const;


private:
    /// 新建事件文件描述符
    static int createEventFd();
    /// 唤醒EventFd
    void wakeup();
    /// 将回调函数放到队列中, 在该Loop所处线程里运行
    void queueLoop(Functor &&functor);
    /// 执行放入当前线程的函数
    void doPendingFunctors();
    /// mWakeUpChannel的回调
    void handleRead();
};


#endif //MYWEBSERVER_EVENTLOOP_H
