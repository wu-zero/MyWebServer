/// \file websource.cpp
///
/// 主函数
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/15.

#include "EventLoop.h"
#include "TcpServer.h"
#include "HttpConnectionManager.h"

//int main()
//{
//    EventLoop loop;
//    TcpServer<HttpConnectionManager> myServer(&loop);
//    myServer.start();
//    loop.loop();
//    return 0;
//}

#include <memory>
#include <iostream>

class A{
public:
    A(){};
    virtual void run(){
        std::cout<< "A"<<std::endl;
    };
};


class B :public A{
public:
    B(){};
    void run() override {
        std::cout <<"B" <<std::endl;
    }
};
int main(){
    A a;
    B b;
    std::shared_ptr<A> c = std::make_shared<B>();
    c->run();
}