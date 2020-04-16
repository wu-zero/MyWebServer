/// \file Timer.h
///
/// 利用timefd实现的定时器, Timer是定时器节点, TimerManager利用优先队列管理Timer
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/30.


#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H

#include <functional>
#include <memory>
#include <queue>


class Channel;
class EventLoop;

class Timer
{
public:
    using TimeStampType = int64_t; // 时间戳，单位为微秒
    using Callback = std::function<void()>; //回调
private:
    TimeStampType mExpiredTime; // 到期时间
    TimeStampType mInterval; // 循环间隔
    Callback mExpireCallback; // 回调函数
    bool mIsRepeat; // 是否重复
public:
    Timer(Callback cb, TimeStampType timeout, TimeStampType interval);
    ~Timer();
    /// 获取信息
    TimeStampType getExpireTime() const;
    bool isRepeat() const;
    /// 执行回调函数
    void run();
    /// 执行重复操作
    void restart(TimeStampType now);
    /// 获取现在时间戳
    static TimeStampType now();
};

/// Timer在优先队列中排序函数
struct TimerCmp
{
    bool operator()(std::shared_ptr<Timer> &a, std::shared_ptr<Timer> &b)
    {
        return a->getExpireTime() > b->getExpireTime();
    }
};


class TimerManager
{
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using SPtrTimer = std::shared_ptr<Timer>;
private:
    int mTimerFd;
    SPtrChannel mTimerFdChannel;
    std::priority_queue<SPtrTimer, std::deque<SPtrTimer>, TimerCmp> mTimerQueue;
public:
    explicit TimerManager(EventLoop* loop);
    ~TimerManager();
    /// 添加计时器
    void addTimer(const Timer::Callback& cb, Timer::TimeStampType timeout, Timer::TimeStampType interval);
private:
    /// 用于Channel的回调
    void handleRead();
    /// 获得到期节点
    std::vector<SPtrTimer> getExpired(Timer::TimeStampType now);
    /// 添加节点, 更改TimerFd时间
    bool insertTimer(const SPtrTimer &timer);
    /// 更新需要循环重复的Timer
    void updateCycleTimer(const std::vector<SPtrTimer>&expired, Timer::TimeStampType now);
};


#endif //MYWEBSERVER_TIMER_H
