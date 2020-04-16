/// \file TcpServer.cpp
///
/// TcpServer的具体实现, 负责管理Acceptor和Connection, 前者负责建立连接, 后者负责每个连接
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.

#include "TcpServer.h"

#include <assert.h>

#include "TcpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"



TcpServer::TcpServer(EventLoop *loop)
        : mEventLoop(loop),
          mPtrAcceptor(new Acceptor(mEventLoop)),
          mConnectionMap(),
          mConnectionCallback(nullptr),
          mMessageCallback(nullptr),
          mWriteCompleteCallback(nullptr)
{
    // 创建Acceptor
    //mPtrAcceptor = std::make_shared<Acceptor>(mEventLoop);
    // 设置Acceptor接到新连接时的回调
    mPtrAcceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    // Acceptor开始
    mPtrAcceptor->start();
}


void TcpServer::setConnectionCallback(const TcpServer::ConnectionCallback &cb)
{
    mConnectionCallback = cb;
}

void TcpServer::setMessageCallback(const TcpServer::MessageCallback &cb)
{
    mMessageCallback = cb;
}

void TcpServer::setWriteCompleteCallback(const TcpServer::WriteCompleteCallback& cb)
{
    mWriteCompleteCallback = cb;
}


void TcpServer::newConnection(int sockFd)
{
    // 创建新连接的相关内容
    SPtrConnection newConnection = std::make_shared<TcpConnection>(mEventLoop, sockFd);
    // 放到管理map中
    mConnectionMap[sockFd] = newConnection;

    // 设置Connection关闭时的回调, Connection关闭时让TcpServer删除与它有关的内容
    newConnection->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 设置TcpConnection回调, 用户层面
    if(mConnectionCallback != nullptr){
        mConnectionCallback(newConnection);
    }
    if(mMessageCallback != nullptr){
        newConnection->setMessageCallback(mMessageCallback);
    }
    if(mWriteCompleteCallback != nullptr){
        newConnection->setWriteCompleteCallback(mWriteCompleteCallback);
    }

    // 调用Connection建立连接时的相关函数
    newConnection->start();
}

void TcpServer::removeConnection(const TcpServer::SPtrConnection &connection)
{
    size_t n = mConnectionMap.erase(connection->getSocketFd());
    assert(n == 1);
}


