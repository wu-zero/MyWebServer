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

#include "Timer.h"

class Channel;
class Epoll;
class TimerManager;


// EventLoop 的主要功能是运行事件循环 EventLoop::loop()
class EventLoop
{
private:
    using SPtrEpoll = std::shared_ptr<Epoll>;
    using SPtrTimerManager = std::shared_ptr<TimerManager>;
    using SptrChannel = std::shared_ptr<Channel>;
    using Functor = std::function<void()>; // pending functor 的类型
private:
    bool mIsQuit;
    SPtrEpoll mEpoll; // Epoll对象
    int mWakeUpEventFd; // 用于唤醒 IO 线程的文件描述符
    SptrChannel mWakeUpChannel; // 用于唤醒 IO 线程的文件描述符对应的Channel
    std::vector<Functor> mPendingFunctors; // 需要在 IO 线程执行的"任务"（函数）
    SPtrTimerManager mTimerManager;
public:
    EventLoop();
    ~EventLoop();

    void loop(); // 运行事件循环
    // Acceptor和Connection的Channel相关
    void updateChannel(Channel *channel); // 添加、更新channel, 主要操作Epoll
    void removeChannel(Channel *channel); // 删除channel, 主要操作Epoll
    //
    void queueLoop(const Functor &functor); // 将回调函数放到队列中, 并在必要时唤醒 IO 线程。
private:
    static int createEventFd(); //新建事件文件描述符
    void wakeup(); //唤醒IO线程
    void doPendingFunctors(); // 调用mPendingFunctors
    
    // mWakeUpChannel的回调
    void handleRead(); // 异步执行
};


#endif //MYWEBSERVER_EVENTLOOP_H
