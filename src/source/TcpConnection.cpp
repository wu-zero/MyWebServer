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
#include <assert.h>

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
    assert(mTcpConnectionState == kStateDisconnected);
    // 关闭socket
    SocketUtils::close(mSocketFd);
}

void TcpConnection::start()
{
    mLoop->runInLoop(std::bind(&TcpConnection::startInLoop,this));
}

void TcpConnection::startInLoop() {
    mLoop->assertInLoopThread();
    mPtrChannel->enableReading();
}

void TcpConnection::shutdown()
{
    mLoop->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
}

void TcpConnection::shutdownInLoop() {
    mLoop->assertInLoopThread();
    SocketUtils::shutdownWrite(mSocketFd);
}

void TcpConnection::close(){
    if (mTcpConnectionState == kStateConnected || mTcpConnectionState == kStateDisconnecting) {
        setStateInLoop(kStateDisconnecting);
        mLoop->runInLoop(std::bind(&TcpConnection::closeInLoop, shared_from_this()));
    }
 }

void TcpConnection::closeInLoop() {
    if (mTcpConnectionState == kStateConnected || mTcpConnectionState == kStateDisconnecting) {
        setStateInLoop(kStateDisconnecting);
        handleClose();
    }
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
    if(mTcpConnectionState == kStateConnected){
        if(mLoop->isInLoopThread()){
            sendInLoop(message);
        }
        else{
            mLoop->runInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), message));
        }
    }
}
void TcpConnection::sendInLoop(const std::string &message) {
    assert(mLoop->isInLoopThread());
    mOutBuffer.append(message);
    if(!mPtrChannel->isWriting())
    {
        mPtrChannel->enableWriting();
    }
}


void TcpConnection::handleRead()
{
    mLoop->assertInLoopThread();
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
    mLoop->assertInLoopThread();
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
                mLoop->runInLoop(std::bind(mWriteCompleteCallback, shared_from_this()));
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
    mLoop->assertInLoopThread();
    std::cout << "TcpConnection::close()" << std::endl;
    assert(mTcpConnectionState == kStateConnected || kStateDisconnecting);
    setStateInLoop(kStateDisconnected);
    // 从Epoll中注销、删除
    mPtrChannel->disableAll();
    mPtrChannel->remove();
    // 通知管理层删了与自己有关的东西
    mCloseCallback(shared_from_this());
}

void TcpConnection::setState(TcpConnection::TcpConnectionState state)
{
    mLoop->runInLoop(std::bind(&TcpConnection::setStateInLoop, this, state));
}

void TcpConnection::setStateInLoop(TcpConnection::TcpConnectionState state) {
    mLoop->assertInLoopThread();
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








