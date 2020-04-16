/// \file websource.cpp
///
/// 主函数
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/15.

#include "MyServer.h"

int main()
{
    EventLoop loop;
    MyServer myServer(&loop);
    myServer.start();
    loop.loop();
    return 0;
}
