/// \file Timer.cpp
///
/// 利用timefd实现的定时器, Timer是定时器节点, TimerManager利用优先队列管理Timer
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/30.


#include "Timer.h"

#include <sys/timerfd.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <utility>

#include "Channel.h"

Timer::Timer(Callback cb, TimeStampType timeout, TimeStampType interval = -1)
        : mExpiredTime(timeout > 0 ? timeout : -1),
          mInterval(interval > 0 ? interval : -1),
          mExpireCallback(std::move(cb)),
          mIsRepeat(interval > 0) {
    assert(timeout > 0);
    mExpiredTime = Timer::now() + timeout;
}

Timer::~Timer() {
}

Timer::TimeStampType Timer::getExpireTime() const {
    return mExpiredTime;
}


void Timer::run() {
    if (mExpireCallback != nullptr) {
        mExpireCallback();
    }
}

bool Timer::isRepeat() const {
    return mIsRepeat;
}

void Timer::restart(Timer::TimeStampType now) {
    if (mIsRepeat) {
        mExpiredTime = now + mInterval;
    }
}

Timer::TimeStampType Timer::now() {
    struct timeval now;
    gettimeofday(&now, nullptr);
    TimeStampType timeNow = (now.tv_sec * 1000 * 1000) + now.tv_usec; // 单位微秒
//    std::cout <<"now.tv_sec" <<now.tv_sec << "now.tv_usec" <<now.tv_usec << std::endl;
//    std::cout << "timeNow: " <<timeNow <<std::endl;
    return timeNow;
}



/// timerfd 操作===============================
namespace TimerFd {
    /// 新建timerfd
    /// \return
    int creatTimerFd() {
        int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerFd < 0) {
            std::cout << "creatTimerFd() error" << std::endl;
        }
        return timerFd;
    }

    /// 读取 timerfd数据
    /// \param timerfd
    /// \param now 读取数据的时刻
    void readTimerFd(int timerfd, Timer::TimeStampType now) {
        uint64_t howmany;
        ssize_t n = read(timerfd, &howmany, sizeof(howmany));
//        std::cout << "TimerQueue::handleRead() "
//                  << howmany << " at " << now << std::endl;
        if (n != sizeof(howmany)) {
            std::cout << "TimerQueue::handleRead() reads "
                      << n << " bytes instead of 8" << std::endl;
        }
    }

    timespec howMuchTimeFromNow(Timer::TimeStampType when) {
        Timer::TimeStampType microseconds = when - Timer::now();
        const int minTime = 100;
        if (microseconds < minTime) {
            microseconds = minTime;
        }
        timespec ts{};
        ts.tv_sec = static_cast<time_t>(microseconds / (1000 * 1000));
        ts.tv_nsec = static_cast<long>((microseconds % (1000 * 1000)) * 1000);
        //std::cout <<"ts.tv_sec: " << ts.tv_sec << " ts.tv_nsec: " << ts.tv_nsec << std::endl;
        return ts;
    }

    void resetTimerFd(int timerfd, Timer::TimeStampType when) {
        itimerspec oldValue{};
        itimerspec newValue{};
        newValue.it_value = howMuchTimeFromNow(when);
        // 通过 timerfd_settime 函数唤醒 IO 线程
        int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
        if (ret) {
            std::cout << "resetTimerFd()" << std::endl;
        }
    }
}


TimerManager::TimerManager(EventLoop *loop)
        : mTimerFd(TimerFd::creatTimerFd()),
          mTimerFdChannel(new Channel(loop, mTimerFd)) {
    // 设置"读"回调函数和可读
    mTimerFdChannel->setReadHandler(std::bind(&TimerManager::handleRead, this));
    mTimerFdChannel->enableReading();
}

TimerManager::~TimerManager() {
    mTimerFdChannel->disableAll();
    mTimerFdChannel->remove();
    close(mTimerFd);
}


void TimerManager::addTimer(const Timer::Callback &cb, Timer::TimeStampType timeout, Timer::TimeStampType interval) {
    if (timeout <= 0) {
        return;
    }
    SPtrTimer newTimer(new Timer(cb, timeout, interval));
    insertTimer(newTimer);
}

void TimerManager::handleRead() {
    // 读超时事件通知
    Timer::TimeStampType now = Timer::now();
    TimerFd::readTimerFd(mTimerFd, now);
    std::vector<std::shared_ptr<Timer>> expired = getExpired(now);

    // 执行事件
    for (const std::shared_ptr<Timer> &it: expired) {
        it->run();
    }

    // 更新循环定时器, 不设置下一个超时时间
    updateCycleTimer(expired, now);

    // 设置下一个超时时间
    if (!mTimerQueue.empty()) {
        auto earliestTimer = mTimerQueue.top();
        auto newExpiredTime = earliestTimer->getExpireTime();
        TimerFd::resetTimerFd(mTimerFd, newExpiredTime);
    }
}

std::vector<std::shared_ptr<Timer>> TimerManager::getExpired(Timer::TimeStampType now) {
    std::vector<SPtrTimer> expired;
    while (!mTimerQueue.empty()) {
        std::shared_ptr<Timer> sPtrTimer = mTimerQueue.top();
        if (sPtrTimer->getExpireTime() <= now) {
            expired.push_back(sPtrTimer);
            mTimerQueue.pop();
        } else {
            break;
        }
        //std::cout<< "mTimerQueue.size()" <<mTimerQueue.size() << std::endl;
    }
    return expired;
}

bool TimerManager::insertTimer(const TimerManager::SPtrTimer &timer) {
    mTimerQueue.push(timer);
    //std::cout << "添加定时器: " << timer->getExpireTime() << std::ends;

    if (timer == mTimerQueue.top()) {
        Timer::TimeStampType expiredTime = timer->getExpireTime();
        TimerFd::resetTimerFd(mTimerFd, expiredTime);
        //std::cout << "更改mTimerFd时间： " << expiredTime << std::endl;
    } else {
        //std::cout << "不更改mTimerFd时间" << std::endl;
    }
}

void TimerManager::updateCycleTimer(const std::vector<TimerManager::SPtrTimer> &expired, Timer::TimeStampType now) {
    for (const SPtrTimer &it: expired) {
        if (it->isRepeat()) {
            it->restart(now);
            mTimerQueue.push(it);
        }
    }
}


