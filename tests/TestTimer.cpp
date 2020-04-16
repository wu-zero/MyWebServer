/// \file TimerTest.cpp
///
/// 定时器测试
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/31.

#include <iostream>

#include "EventLoop.h"
#include "Timer.h"


static void fun(int funId)
{
    std::cout << "处理事件: " << funId << std::endl;
}


int main()
{
    EventLoop loop;
    TimerManager timerManager(&loop);
    timerManager.addTimer(std::bind(fun,1),10000000,0);
    timerManager.addTimer(std::bind(fun,2),5000000,0);
    timerManager.addTimer(std::bind(fun,3),20000000,0);
    timerManager.addTimer(std::bind(fun,4),15000000,0);
    timerManager.addTimer(std::bind(fun,5),15000000,5000000);
    loop.loop();
}