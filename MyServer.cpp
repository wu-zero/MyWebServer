/// \file MyServer.cpp
///
/// ClassB detailed file description.
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/16.


#include "MyServer.h"
#include <iostream>

MyServer::MyServer(EventLoop *pLoop)
        : mPtrLoop(pLoop),
          mServer(pLoop)
{
    mServer.setUser(this);
}

MyServer::~MyServer()
{
}

void MyServer::start()
{
    mServer.start();
    mServer.setUser(this);
}

void MyServer::onConnection(TcpConnection *pCon)
{
    std::cout << "MyServer onConnection" << std::endl;
}

void MyServer::onMessage(TcpConnection *pCon, Buffer *pBuf)
{
    while (pBuf->readableBytes() > 15)
    {
        std::string message = pBuf->retrieveAsString(15);
        std::cout << "MyServer Read15:" << message << std::endl;
        //pCon->send(message + "\n");
    }
}

void MyServer::onWriteComplete(TcpConnection *pCon)
{
    std::cout << "onWriteComplate" << std::endl;
}
