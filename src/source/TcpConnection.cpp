/// \file TcpConnection.cpp
///
/// 每个新连接对应一个TcpConnection对象, 回调执行socket的相应读写事件, 通过EventLoop异步执行相应操作
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "TcpConnection.h"

#include "socket_utils.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "Channel.h"
#include "IUser.h"
#include "EventLoop.h"

TcpConnection::TcpConnection(EventLoop *eventLoop, int socketFd)
        : mLoop(eventLoop),
          mSocketFd(socketFd),
          mPtrChannel(nullptr),
          mOutBuffer(),
          mInBuffer(),
          mCloseCallback(),
          mPtrUser(nullptr)
{
    mPtrChannel = std::make_shared<Channel>(eventLoop, socketFd);
    mPtrChannel->setReadHandler(std::bind(&TcpConnection::handleRead, this));
    mPtrChannel->setWriteHandler(std::bind(&TcpConnection::handleWrite, this));
    mPtrChannel->setCloseHandler(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection()
{
   // 资源释放在handleClose()里
}

void TcpConnection::start()
{
    mPtrChannel->enableReading();
    userConnectCallback();
}

void TcpConnection::send(const std::string &message)
{
    int n = 0;
    int sockFd = mPtrChannel->getSocketFd();
    if(mOutBuffer.readableBytes() == 0)
    {
        n = writen(sockFd, message.c_str(), message.size());
        if(n < 0)
            std::cout << "write error" << std::endl;
        if(n == static_cast<int>(message.size())) // 写完调用用户写回调
            mLoop->queueLoop(std::bind(&TcpConnection::userWriteCallback, this));
    }

    if( n < static_cast<int>(message.size()))
    {
        mOutBuffer.append(message.substr(n, message.size()));
        if(!mPtrChannel->isWriting())
        {
            mPtrChannel->enableWriting();
        }
    }
}


void TcpConnection::setCloseCallback(const TcpConnection::CloseCallback &cb)
{
    mCloseCallback = cb;
}


void TcpConnection::handleRead()
{
    int sockFd = mPtrChannel->getSocketFd();
    if (sockFd < 0)
    {
        std::cout << "TcpConnection::handleRead sockFd < 0 error " << std::endl;
        return;
    }
    const int READ_BUFF_MAX = 65536;
    char buff[READ_BUFF_MAX];
    bzero(buff, sizeof(buff));

    int readLength = readn(sockFd, buff, READ_BUFF_MAX);

    if (readLength < 0)
    {
        std::cout << "TcpConnection::handleRead readlength < 0 error" << std::endl;
    }
    else if(readLength == 0)
    {
        // 有请求出现但是读不到数据
        // 对端可能调用shutdown或close
        // 统一按照对端已经关闭处理
        handleClose();
    }
    else
    {
        std::string buffString(buff, readLength);
        mInBuffer.append(buffString);
        userReadCallback(&mInBuffer); // 用户消息读取成功回调函数
    }

}

void TcpConnection::handleWrite()
{
    int sockFd = mPtrChannel->getSocketFd();
    if (sockFd < 0)
    {
        std::cout << "TcpConnection::handleRead sockFd < 0 error " << std::endl;
        return;
    }
    if(mPtrChannel->isWriting())
    {
        int n = writen(sockFd, mOutBuffer.peek(), mOutBuffer.readableBytes());
        if(n>0)
        {
            std::cout << "write " << n << " bytes data" << std::endl;
        }
        mOutBuffer.retrieve(n);
        if(mOutBuffer.readableBytes() == 0)
        {
            mPtrChannel->disableWriting();
            mLoop->queueLoop(std::bind(&TcpConnection::userWriteCallback, this)); // 用户消息写成功回调函数
        }
    }
}


void TcpConnection::handleClose()
{
    std::cout << "close" << std::endl;
    mPtrChannel->disableAll();
    mPtrChannel->remove(); //从Epoll中注销
    if(close(mSocketFd) < 0)
    {
        std::cout <<"close error" << std::endl;
    }

    //通知管理层删了与自己有关的东西
    if(mCloseCallback){
        mCloseCallback(std::shared_ptr<TcpConnection>(this));
    }
}


void TcpConnection::setUser(IUser *ptrUser)
{
    mPtrUser = ptrUser;
}


void TcpConnection::userReadCallback(Buffer* pBuf){
    if(mPtrUser != nullptr)
    {
        mPtrUser->onMessage(this, pBuf);
    }
}

void TcpConnection::userWriteCallback()
{
    if(mPtrUser != nullptr){
        mPtrUser->onWriteComplete(this);
    }

}

void TcpConnection::userConnectCallback()
{
    if(mPtrUser != nullptr)
    {
        mPtrUser->onConnection(this);
    }
}



