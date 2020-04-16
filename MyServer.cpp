/// \file MyServer.cpp
///
/// MyServer包含一个TcpServer, 每个TcpConnection通过weak_ptr与一个继承了IHolder类的HttpHandler互相绑定来实现Http功能。
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/5.


#include "MyServer.h"
#include <iostream>
#include "HttpHandler.h"
#include "IHolder.h"
#include <functional>
#include <assert.h>

const Timer::TimeStampType MyServer::kKeepAliveTime = HttpHandler::kKeepAliveTime*1000000;
const Timer::TimeStampType MyServer::kExpireEventCircleInterval = 1000000;

MyServer::MyServer(EventLoop *pLoop)
        : mPtrLoop(pLoop),
          mTimerManager(pLoop),
          mServer(pLoop)
{
    mTimerManager.addTimer(std::bind(&MyServer::handleExpireEvent, this),kKeepAliveTime,kExpireEventCircleInterval);
    mServer.setConnectionCallback(std::bind(&MyServer::onConnection, this, std::placeholders::_1));
    mServer.setMessageCallback(std::bind(&MyServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

MyServer::~MyServer()
{
}

void MyServer::start()
{
    mServer.start();
}

void MyServer::onConnection(const SPtrTcpConnection &sPtrTcpConnection)
{
    std::cout << "MyServer onConnection" << std::endl;

    SPtrHttpConnection newHttpConnection = std::make_shared<HttpHandler>();

    sPtrTcpConnection->setHolder(std::dynamic_pointer_cast<IHolder>(newHttpConnection));
    newHttpConnection->setTcpConnection(sPtrTcpConnection); // 弱

    mTimerNodeList.emplace_back(newHttpConnection, Timer::now() + 15000000);
    std::cout <<"onConnection: "<< Timer::now() + 15000000 <<std::endl;
    mTimerNodeMap[newHttpConnection] = --mTimerNodeList.end();
}

void MyServer::onMessage(const MyServer::SPtrTcpConnection &sPtrTcpConnection, Buffer *pBuf)
{
    std::cout << "onMessage" << std::endl;
    WPtrIContext iContext = sPtrTcpConnection->getHolder();
    if (iContext.expired())
    {   return;
    }
    std::shared_ptr<HttpHandler> httpConnection = std::dynamic_pointer_cast<HttpHandler>(iContext.lock());
    std::string messageIn = pBuf->retrieveAllAsString();
    httpConnection->handleHttpReq(messageIn);

    updateKeepAlive(httpConnection);
    std::string messageOut = httpConnection->responseRequest();
    httpConnection->reset();
    sPtrTcpConnection->send(messageOut);
}

void MyServer::onWriteComplete(const MyServer::SPtrTcpConnection &sPtrTcpConnection)
{
    std::cout << "onWriteComplete" << std::endl;
}

void MyServer::updateKeepAlive(const MyServer::SPtrHttpConnection &sPtrHttpConnection)
{
    WPtrTcpConnection wPtrTcpConnection = sPtrHttpConnection->getTcpConnection();
    // 获得http对应的connection
    if (wPtrTcpConnection.expired())
    {
        return;
    }
    SPtrTcpConnection sPtrTcpConnection = wPtrTcpConnection.lock();
    // 处理http对应的tcp的连接
    if (!sPtrHttpConnection->isKeepAlive())                   // 如果http不是长连接
    {
        std::cout << "!sPtrHttpConnection->isKeepAlive()" << std::endl;
        sPtrTcpConnection->setState(TcpConnection::kStateDisconnecting);
        //sPtrTcpConnection->shutdown();
        return;
    } else                                                    // 如果http是长连接
    {
        std::cout << "sPtrHttpConnection->isKeepAlive()" << std::endl;

        // 如果有当前http对应的时间节点， list、map里删除
        if (mTimerNodeMap.count(sPtrHttpConnection))
        {
            auto position = mTimerNodeMap[sPtrHttpConnection];
            mTimerNodeList.erase(position);
            mTimerNodeMap.erase(sPtrHttpConnection);
        }

        // 新建时间节点
        Timer::TimeStampType now = Timer::now();
        mTimerNodeList.emplace_back(sPtrHttpConnection, now + 600000000);
        std::cout <<"updateKeepAlive: "<< now + 600000000 <<std::endl;
        mTimerNodeMap[sPtrHttpConnection] = --mTimerNodeList.end();
    }
}


void MyServer::handleExpireEvent()
{
    Timer::TimeStampType now = Timer::now();
    for (auto it = mTimerNodeList.begin(); it !=mTimerNodeList.end();)
    {
        if(it->second > now)
        {
            break;
        }
        std::cout << "到期时间: " <<it->second << " 现在时间: " << now<<std::endl;
        SPtrHttpConnection sPtrHttpConnection = it->first;
        assert(it==mTimerNodeMap[sPtrHttpConnection]);
        mTimerNodeList.erase(it++);
        mTimerNodeMap.erase(sPtrHttpConnection);

        WPtrTcpConnection wPtrTcpConnection = sPtrHttpConnection->getTcpConnection();
        // 获得http对应的connection
        if (wPtrTcpConnection.expired())
        {
            return;
        }
        SPtrTcpConnection sPtrTcpConnection = wPtrTcpConnection.lock();
        sPtrTcpConnection->close();
    }
}

