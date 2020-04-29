/// \file ConnectionManager.cpp
///
/// Manager基类, 只负责基本的Connection的管理了, 即：添加和删除
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/28.


#include "ConnectionManager.h"

#include <assert.h>
#include <iostream>
#include <memory>


#include "EventLoop.h"
#include "TcpConnection.h"

ConnectionManager::ConnectionManager(EventLoop *loop)
:mLoop(loop)
{

}

ConnectionManager::~ConnectionManager() {

}


void ConnectionManager::addConnection(const ConnectionManager::SPtrConnection& sPtrConnection) {
    mLoop->runInLoop(std::bind(&ConnectionManager::addConnectionInLoop, this, sPtrConnection));
}

void ConnectionManager::addConnectionInLoop(const ConnectionManager::SPtrConnection &sPtrConnection) {
    mLoop->assertInLoopThread();
    mConnectionMap[sPtrConnection->getSocketFd()] = sPtrConnection;
}

void ConnectionManager::deleteConnection(const ConnectionManager::SPtrConnection &sPtrConnection) {
    mLoop->runInLoop(std::bind(&ConnectionManager::deleteConnectionInLoop, this, sPtrConnection));
}

void ConnectionManager::deleteConnectionInLoop(const ConnectionManager::SPtrConnection &sPtrConnection) {
    mLoop->assertInLoopThread();
    size_t n =mConnectionMap.erase(sPtrConnection->getSocketFd());
    assert(n == 1);
}

