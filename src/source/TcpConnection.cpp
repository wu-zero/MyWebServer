/// \file TcpConnection.cpp
///
/// 每个新连接对应一个TcpConnection对象，回调执行socket的读写事件
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "TcpConnection.h"

#include <iostream>
#include <cstring>

TcpConnection::TcpConnection(int epollFd, int socketFd)
        : mEpollFd(epollFd),
          mSocketFd(socketFd)
{
    mPtrChannel = std::make_shared<Channel>(mEpollFd);
    mPtrChannel->setSocketFd(mSocketFd);
    mPtrChannel->enableReading();
    mPtrChannel->setReadHandler(std::bind(&TcpConnection::handleRead, this));
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::handleRead()
{
    std::cout << "TcpConnection::handleRead" << std::endl;
    const int  READ_BUFF_MAX = 1024;
    char buff[READ_BUFF_MAX];
    bzero(buff, sizeof(buff));
    int readLength = readn(mSocketFd, buff, READ_BUFF_MAX);
    std::cout << "Receive " << readLength << " byte from Client  "
              << " Receive: asda" << buff << std::endl;
}

void TcpConnection::handleWrite()
{
}

void TcpConnection::handleClose()
{
}


