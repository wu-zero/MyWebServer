/// \file Acceptor.cpp
///
/// 负责处理新连接，监听到有Read，就执行Read回调创建一个新的Accept，
/// 同时执行NewConnection回调，让Server执行相应操作（创建TcpConnection对象）
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "Acceptor.h"

#include <memory>
#include <iostream>
#include <unistd.h>

#include "socket_utils.h"
#include "Channel.h"


Acceptor::Acceptor(EventLoop *loop)
    : mPtrLoop(loop),
      mListenFd(-1),
      mPtrAcceptChannel(nullptr),
      mNewConnectionCallback(nullptr)
{
}

Acceptor::~Acceptor()
{
    close(mListenFd);
    mPtrAcceptChannel->disableAll();
    mPtrAcceptChannel->remove(); //从Epoll中注销
}

void Acceptor::start()
{
    mListenFd = creatSocketAndListen();
    if (mListenFd < 0){
        std::cout << "creatSocketAndListen failed" << std::endl;
        abort();
    }

    mPtrAcceptChannel = std::make_shared<Channel>(mPtrLoop, mListenFd);
    mPtrAcceptChannel->enableReading();
    mPtrAcceptChannel->setReadHandler(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::handleRead()
{
    std::cout << "Acceptor::handleRead" << std::endl;
    int connFd = creatNewAccept(mListenFd);
    //setSocketNonBlock(connFd);

    // 执行新连接到来时的回调
    if(mNewConnectionCallback){
        mNewConnectionCallback(connFd);
    }
}

void Acceptor::setNewConnectionCallback(const Acceptor::NewConnectionCallback &cb)
{
    mNewConnectionCallback = cb;
}







