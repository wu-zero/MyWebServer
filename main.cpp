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

int main()
{
    EventLoop loop;
    TcpServer<HttpConnectionManager> myServer(&loop);
    myServer.start();
    loop.loop();
    return 0;
}

