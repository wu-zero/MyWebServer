/// \file TestTcpServer.cpp
///
/// Tcp服务端测试
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/15.


#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"


using SPtrTcpConnection = std::shared_ptr<TcpConnection>;

// 接收socket后的回调
static void onMessage(const SPtrTcpConnection &sPtrTcpConnection, Buffer *pBuf){
    std::string message = pBuf->retrieveAllAsString();
    std::cout << "onMessage: " <<  message << std::endl;
}

int main()
{
    EventLoop loop;
    TcpServer myTcpServer(&loop);
    myTcpServer.start();
    myTcpServer.setMessageCallback(onMessage);
    loop.loop();
    return 0;
}

