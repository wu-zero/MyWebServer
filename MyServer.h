/// \file MyServer.h
///
/// MyServer包含一个TcpServer, 通过覆盖IUser接口, 实现一个服务器
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/16.S


#ifndef MYWEBSERVER_MYSERVER_H
#define MYWEBSERVER_MYSERVER_H


#include "IUser.h"
#include "EventLoop.h"
#include "TcpServer.h"

class MyServer: public IUser
{
private:
    EventLoop *mPtrLoop;
    TcpServer mServer;
public:
    explicit MyServer(EventLoop *pLoop);
    ~MyServer();
    void start();

    // IUser接口部分
    void onConnection(TcpConnection *pCon) override;
    void onMessage(TcpConnection* pCon, Buffer* pBuf) override;
    void onWriteComplete(TcpConnection* pCon) override;
};


#endif //MYWEBSERVER_MYSERVER_H
