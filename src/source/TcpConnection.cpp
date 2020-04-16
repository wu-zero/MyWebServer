/// \file TcpConnection.cpp
///
/// 每个新连接对应一个TcpConnection对象, 回调执行socket的相应读写事件, 通过EventLoop异步执行相应操作
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "TcpConnection.h"

#include "SocketUtils.h"
#include <iostream>
#include <cstring>
#include <utility>

#include "Channel.h"
#include "EventLoop.h"

const int READ_BUFF_MAX = 65536;

TcpConnection::TcpConnection(EventLoop *eventLoop, int socketFd)
        : mLoop(eventLoop),
          mSocketFd(socketFd),
          mPtrChannel(nullptr),
          mOutBuffer(),
          mInBuffer(),
          mTcpConnectionState(kStateConnected),
          mCloseCallback(),
          mWPtrContext(),
          mConnectionCallback(),
          mMessageCallback(),
          mWriteCompleteCallback()
{
    mPtrChannel = std::make_shared<Channel>(eventLoop, socketFd);
    // 设置自己Channel的回调
    mPtrChannel->setReadHandler(std::bind(&TcpConnection::handleRead, this));
    mPtrChannel->setWriteHandler(std::bind(&TcpConnection::handleWrite, this));
    mPtrChannel->setCloseHandler(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection()
{
    std::cout << "TcpConnection::~TcpConnection()" <<std::endl;
    // 关闭socket
    SocketUtils::close(mSocketFd);
}

void TcpConnection::start()
{
    mPtrChannel->enableReading();
}

void TcpConnection::shutdown()
{
    SocketUtils::shutdownWrite(mSocketFd);
}

void TcpConnection::close()
{
    std::cout << "TcpConnection::close()" << std::endl;
    // 从Epoll中注销、删除
    mPtrChannel->disableAll();
    mPtrChannel->remove();
    // 通知管理层删了与自己有关的东西
    mCloseCallback(shared_from_this());
}

void TcpConnection::setCloseCallback(const TcpConnection::CloseCallback &cb)
{
    mCloseCallback = cb;
}


void TcpConnection::setHolder(WPtrIHolder wPtrIHolder)
{
    mWPtrContext = std::move(wPtrIHolder);
}

TcpConnection::WPtrIHolder TcpConnection::getHolder()
{
    return mWPtrContext;
}

void TcpConnection::setConnectionCallback(const TcpConnection::ConnectionCallback &cb)
{
    mConnectionCallback = cb;
}

void TcpConnection::setMessageCallback(const TcpConnection::MessageCallback &cb)
{
    mMessageCallback = cb;
}

void TcpConnection::setWriteCompleteCallback(const TcpConnection::WriteCompleteCallback &cb)
{
    mWriteCompleteCallback = cb;
}


void TcpConnection::send(const std::string &message)
{
    std::cout << "TcpConnection::send()" << std::endl;
//    int n = 0;
//    int sockFd = mPtrChannel->getFd();
////    if(mOutBuffer.readableBytes() == 0)
////    {
////        n = writen(sockFd, message.c_str(), message.size());
////        if(n < 0)
////            std::cout << "write error" << std::endl;
////        if(n == static_cast<int>(message.size())) // 写完调用用户写回调
////        {
////            if(mWriteCompleteCallback != nullptr){
////                mLoop->queueLoop(std::bind(&TcpConnection::mWriteCompleteCallback, shared_from_this()));
////            }
////
////        }
////
////    }
////
//    if( n < static_cast<int>(message.size()))
//    {
//        mOutBuffer.append(message.substr(n, message.size()));
//        if(!mPtrChannel->isWriting())
//        {
//            mPtrChannel->enableWriting();
//        }
//    }
    mOutBuffer.append(message);
    if(!mPtrChannel->isWriting())
    {
        mPtrChannel->enableWriting();
    }
}

void TcpConnection::handleRead()
{
    std::cout << "TcpConnection::handleRead()" << std::endl;
    char buff[READ_BUFF_MAX];
    bzero(buff, sizeof(buff));
    int readLength = SocketUtils::readn(mSocketFd,buff,READ_BUFF_MAX);

    if (readLength < 0)        // 读出错
    {
        std::cout << "TcpConnection::handleRead(): readLength < 0" << std::endl;
    }
    else if(readLength == 0)   // 对端关闭
    {
        std::cout << "TcpConnection::handleRead(): readLength == 0" <<std::endl;
        // 有请求出现但是读不到数据
        // 对端可能调用shutdown或close
        // 统一按照对端已经关闭处理
        mTcpConnectionState = kStateDisconnecting;
        handleClose();
    }
    else                         // 正常
    {
        std::string buffString(buff, readLength);
        mInBuffer.append(buffString);
        // 用户消息读取成功回调函数
        if (mMessageCallback != nullptr){
            mMessageCallback(shared_from_this(), &mInBuffer);
        }
    }
}

void TcpConnection::handleWrite()
{
    int sockFd = mPtrChannel->getFd();

    if(mPtrChannel->isWriting())
    {
        int n = SocketUtils::writen(sockFd, mOutBuffer.peek(), mOutBuffer.readableBytes());
        mOutBuffer.retrieve(n);
        if(mOutBuffer.readableBytes() == 0) // 写完了
        {
            mPtrChannel->disableWriting();
            if(mWriteCompleteCallback)
            {
                mWriteCompleteCallback(shared_from_this());
            }
            // 不再写了, 关闭写, 发送FIN
            if(mTcpConnectionState == kStateDisconnecting){
                std::cout << "kStateDisconnecting and TcpConnection::handleWrite()" << std::endl;
                shutdown();
            }
        }
    }
}

void TcpConnection::handleClose()
{
    close();
}

void TcpConnection::setState(TcpConnection::TcpConnectionState state)
{
    mTcpConnectionState = state;
}

TcpConnection::TcpConnectionState TcpConnection::getState()
{
    return mTcpConnectionState;
}


int TcpConnection::getSocketFd() const
{
        return mSocketFd;
}


