/// \file Acceptor.cpp
///
/// 负责处理新连接，监听到有Read，就执行Read回调创建一个新的Accept，
/// 同时执行NewConnection回调，让Server执行相应操作（创建TcpConnection对象）
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "Acceptor.h"

#include <string.h>
#include <memory>
#include <iostream>
#include <unistd.h>

#include "SocketUtils.h"
#include "Channel.h"


Acceptor::Acceptor(EventLoop *loop)
    : mPtrLoop(loop),
      mListenFd(-1),
      mSPtrAcceptChannel(nullptr),
      mNewConnectionCallback(nullptr)
{
    mListenFd = SocketUtils::creatSocketAndListen(55555);
    if (mListenFd < 0){
        std::cout << "creatSocketAndListen failed" << std::endl;
        abort();
    }
    mSPtrAcceptChannel = std::make_shared<Channel>(mPtrLoop, mListenFd);
    // 设置自己Channel的回调
    mSPtrAcceptChannel->setReadHandler(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    mSPtrAcceptChannel->disableAll(); // 关闭监听
    mSPtrAcceptChannel->remove(); // 从Epoll中删除
    close(mListenFd);
}

void Acceptor::start()
{
    std::cout << "Acceptor::start()" << std::endl;
    mSPtrAcceptChannel->enableReading();
}

void Acceptor::setNewConnectionCallback(const Acceptor::NewConnectionCallback &cb)
{
    mNewConnectionCallback = cb;
}

void Acceptor::handleRead()
{
//    std::cout << "Acceptor::handleRead()" << std::endl;
    int connFd = SocketUtils::creatNewAccept(mListenFd);
    if (connFd > 0){
        // 执行新连接到来时的回调
        if(mNewConnectionCallback){
            mNewConnectionCallback(connFd);
        }
        else{
            close(connFd);
        }
    }
    else{
        std::cout << "Acceptor::handleRead() err , with msg is:" << strerror(errno) << std::endl;
    }

}





