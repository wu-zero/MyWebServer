/// \file TcpConnection.h
///
/// 每个新连接对应一个TcpConnection对象，回调执行socket的相应读写事件
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER2_TCPCONNECTION_H
#define MYWEBSERVER2_TCPCONNECTION_H

#include "Channel.h"
#include "socket_utils.h"
#include <memory>

class TcpConnection
{
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using ReadCallback = std::function<void()>; //该连接可读时的回调函数
private:
    int mEpollFd;
    int mSocketFd;
    SPtrChannel mPtrChannel;
public:
    TcpConnection(int epollFd, int socketFd);
    ~TcpConnection();

    void handleRead();
    void handleWrite();
    void handleClose();
};


#endif //MYWEBSERVER2_TCPCONNECTION_H
