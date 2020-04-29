/// \file TestEventLoopThreadPool.cpp
///
/// 测试EventLoopThreadPool
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/27.

#include <iostream>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

int main(){
    EventLoop loop;
    EventLoopThreadPool myEventLoopThreadPool(&loop, 4);
    myEventLoopThreadPool.start();
    for(int i=0;i<8;i++){
        auto loop = myEventLoopThreadPool.getNextLoop();
        std::cout <<loop->getThreadId()<<std::endl;
    }
}
