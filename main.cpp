/// \file source.cpp
/// \brief
///
/// A_wrong detailed file description.
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/7.

#include "MyServer.h"

int main()
{
    EventLoop loop;
    MyServer myServer(&loop);
    myServer.start();
    loop.loop();
    return 0;
}


