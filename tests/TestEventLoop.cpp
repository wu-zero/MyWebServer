/// \file TestEventLoop.cpp
///
/// 测试EventLoop
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/26.


#include "EventLoop.h"

#include <iostream>
#include <thread>

void fun1(){
    std::cout << "eventLoop.runInLoop()函数任务执行id: " << std::this_thread::get_id() << std::endl;
}

void fun2(EventLoop &eventLoop){
    std::cout << "eventLoop.runInLoop函数调用线程id: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    eventLoop.runInLoop(&fun1);
}

void fun3(EventLoop &eventLoop){
    std::cout << "eventLoop.quit()函数调用线程id: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    eventLoop.quit();
}

int main(){
    EventLoop myEventLoop;
    std::cout << "主函数线程id: " << std::this_thread::get_id() << std::endl
    <<"myEventLoop所处线程id: "<< myEventLoop.getThreadId()<<std::endl;
    // 测试runInLoop
    std::thread t1(&fun2, std::ref(myEventLoop));
    // 测试quit
    std::thread t2(&fun3, std::ref(myEventLoop));

    myEventLoop.loop();
    t1.join();
    t2.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
