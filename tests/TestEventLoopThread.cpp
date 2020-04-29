/// \file TestEventLoopThread.cpp
///
/// 测试EventLoopThread
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/27.

#include <iostream>
#include <thread>

#include "EventLoop.h"
#include "EventLoopThread.h"

void fun1(){
    std::cout << "eventLoop.runInLoop()函数任务执行id: " << std::this_thread::get_id() << std::endl;
}

void fun2(EventLoop &eventLoop){
    std::cout << "eventLoop.runInLoop()函数调用线程id: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    eventLoop.runInLoop(&fun1);
}

void fun3(EventLoop &eventLoop){
    std::cout << "eventLoop.quit()函数调用线程id: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    eventLoop.quit();
}

int main(){
    EventLoopThread myEventLoopThread;
    EventLoop *myEventLoop = myEventLoopThread.startLoop();
    std::cout << "主函数线程id: " << std::this_thread::get_id() << std::endl
          <<"myEventLoop所处线程id: "<< myEventLoop->getThreadId()<<std::endl;

    // 测试runInLoop
    fun2(*myEventLoop);

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::cout << "关闭Eventoop" << std::endl;
    // 测试quit
    fun3(*myEventLoop);


    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::cout << "结束主进程" << std::endl;
}


