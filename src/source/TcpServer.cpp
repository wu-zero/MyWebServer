/// \file TcpServer.cpp
///
/// Server的具体实现，epoll监听一个socket，
/// 对于Client请求，用Acceptor建立新的socket连接，epoll中增加新的监听socket
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.



#include <iostream>
#include <vector>

#include "TcpConnection.h"
#include "TcpServer.h"


TcpServer::TcpServer()
        : mEpollFd(-1),
        mPtrAcceptor(nullptr)
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::newConnection(int sockfd)
{
    mConnections[sockfd] = std::make_shared<TcpConnection>(mEpollFd, sockfd);
}

void TcpServer::start()
{
    mEpollFd = epoll_create(MAX_LISTEN);
    if (mEpollFd <= 0)
        std::cout << "epoll_create error, errno:" << mEpollFd << std::endl;

    mPtrAcceptor = std::make_shared<Acceptor>(mEpollFd);
    mPtrAcceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
    mPtrAcceptor->start();
    for (;;)
    {
        std::vector<Channel *> channels;
        int fds = epoll_wait(mEpollFd, mEvents, MAX_EVENTS, -1); //epoll_wait
        if (fds == -1)
        {
            std::cout << "epoll_wait error, errno:" << errno << std::endl;
            break;
        }
        for (int i = 0; i < fds; i++) //找出要回调的Channel
        {
            Channel *pChannel = static_cast<Channel *>(mEvents[i].data.ptr);
            pChannel->setRevents(mEvents[i].events);
            channels.push_back(pChannel);
        }

        std::vector<Channel *>::iterator it; //执行回调
        for (it = channels.begin(); it != channels.end(); ++it)
        {
            (*it)->handleEvent();
        }
    }
}


