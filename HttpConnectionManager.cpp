/// \file HttpConnectionManager.cpp
///
/// 每个SubReactor里有一个HttpConnectionManager, 管理TcpConnection和HttpHandler
/// 每个TcpConnection通过weak_ptr与一个继承了IHolder类的HttpHandler互相绑定来实现Http功能。
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/29.



#include "HttpConnectionManager.h"

#include <iostream>
#include <assert.h>
#include <functional>

#include "EventLoop.h"
#include "TcpConnection.h"

const Timer::TimeStampType HttpConnectionManager::kKeepAliveTime = HttpHandler::kKeepAliveTime*1000000;
const Timer::TimeStampType HttpConnectionManager::kExpireEventCircleInterval = 1000000;

HttpConnectionManager::HttpConnectionManager(EventLoop *loop)
        : ConnectionManager(loop),
          mTimerManager(mLoop)
{
    mTimerManager.addTimer(std::bind(&HttpConnectionManager::handleExpireEvent, this), kKeepAliveTime, kExpireEventCircleInterval);
}

HttpConnectionManager::~HttpConnectionManager() {

}

void HttpConnectionManager::addConnection(const ConnectionManager::SPtrConnection &sPtrConnection) {
    mLoop->runInLoop(std::bind(&HttpConnectionManager::addConnectionInLoop,this, sPtrConnection));
}

void HttpConnectionManager::addConnectionInLoop(const ConnectionManager::SPtrConnection &sPtrTcpConnection) {
    // 调用基类
    ConnectionManager::addConnectionInLoop(sPtrTcpConnection);
    // 当前类相关
    sPtrTcpConnection->setMessageCallback(std::bind(&HttpConnectionManager::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    sPtrTcpConnection->setWriteCompleteCallback(std::bind(&HttpConnectionManager::onWriteComplete, this, std::placeholders::_1));

    SPtrHttpConnection newHttpConnection = std::make_shared<HttpHandler>();

    sPtrTcpConnection->setHolder(std::dynamic_pointer_cast<IHolder>(newHttpConnection));
    newHttpConnection->setTcpConnection(sPtrTcpConnection); // 弱

    mTimerNodeList.emplace_back(newHttpConnection, Timer::now() + kKeepAliveTime);
    std::cout << "onConnection: " << Timer::now() << std::endl;
    mTimerNodeMap[newHttpConnection] = --mTimerNodeList.end();
}

void HttpConnectionManager::deleteConnection(const ConnectionManager::SPtrConnection &sPtrConnection) {
    deleteConnectionInLoop(sPtrConnection);
}

void HttpConnectionManager::deleteConnectionInLoop(const ConnectionManager::SPtrConnection &sPtrConnection) {
    ConnectionManager::deleteConnectionInLoop(sPtrConnection);
}

void HttpConnectionManager::onMessage(const HttpConnectionManager::SPtrTcpConnection &sPtrTcpConnection, Buffer *pBuf)
{
    mLoop->assertInLoopThread();
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

void HttpConnectionManager::onWriteComplete(const HttpConnectionManager::SPtrTcpConnection &sPtrTcpConnection)
{
    mLoop->assertInLoopThread();
    std::cout << "onWriteComplete" << std::endl;
}

void HttpConnectionManager::updateKeepAlive(const HttpConnectionManager::SPtrHttpConnection &sPtrHttpConnection)
{
    mLoop->assertInLoopThread();
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
        mTimerNodeList.emplace_back(sPtrHttpConnection, now + kKeepAliveTime);
        std::cout << "updateKeepAlive: " << now + kKeepAliveTime << std::endl;
        mTimerNodeMap[sPtrHttpConnection] = --mTimerNodeList.end();
    }
}

void HttpConnectionManager::handleExpireEvent()
{
    mLoop->assertInLoopThread();
    Timer::TimeStampType now = Timer::now();
    for (auto it = mTimerNodeList.begin(); it !=mTimerNodeList.end();)
    {
        if(it->second > now)
        {
            break;
        }
        std::cout << "到期时间: " <<it->second << " 现在时间: " << now<<std::endl;
        SPtrHttpConnection sPtrHttpConnection = it->first;
        assert(it == mTimerNodeMap[sPtrHttpConnection]);
        mTimerNodeList.erase(it++);
        mTimerNodeMap.erase(sPtrHttpConnection);

        WPtrTcpConnection wPtrTcpConnection = sPtrHttpConnection->getTcpConnection();
        // 获得http对应的connection
        if (wPtrTcpConnection.expired())
        {
            continue;
        }
        SPtrTcpConnection sPtrTcpConnection = wPtrTcpConnection.lock();
        sPtrTcpConnection->close();

    }
}