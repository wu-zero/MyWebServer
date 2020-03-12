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

Acceptor::Acceptor(int epollFd)
    :mEpollFd(epollFd),
    mListenFd(creatSocketAndListen()),
    mPtrAcceptChannel(new Channel(epollFd))
{
    mPtrAcceptChannel->setSocketFd(mListenFd);
    if (mListenFd < 0){
        std::cout << "creatSocketAndListen failed" << std::endl;
        abort();
    }
    if (setSocketNonBlock(mListenFd) < 0)
    {
        perror("setSocketNonBlock failed");
        abort();
    }
}

Acceptor::~Acceptor()
{
}

void Acceptor::start()
{
    mPtrAcceptChannel->enableReading();
    mPtrAcceptChannel->setReadHandler(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::handleRead()
{
    std::cout << "Acceptor::handleRead" << std::endl;
    int connFd = creatNewAccept(mListenFd);
    setSocketNonBlock(connFd);

    mNewConnectionCallback(connFd);
}

void Acceptor::handleWrite()
{
}

void Acceptor::setNewConnectionCallback(const Acceptor::NewConnectionCallback &cb)
{
    mNewConnectionCallback = cb;
}







