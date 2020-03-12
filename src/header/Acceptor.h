/// \file Acceptor.h
///
/// 负责处理新连接，监听到有Read，就执行Read回调创建一个新的Accept，
/// 同时执行NewConnection回调，让Server执行相应操作（创建TcpConnection对象）
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER2_ACCEPTOR_H
#define MYWEBSERVER2_ACCEPTOR_H


#include "Channel.h"
#include "socket_utils.h"
#include <memory>


class Acceptor
{
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using NewConnectionCallback = std::function<void(int newConnectFd)>;
private:
    int mEpollFd; // epoll文件描述符
    int mListenFd; // 服务器监听的socket的文件描述符
    SPtrChannel mPtrAcceptChannel; // 观察 服务器监听的socket 的事件
    NewConnectionCallback mNewConnectionCallback;   // 新连接到来时的回调函数
public:
    explicit Acceptor(int epollFd);
    ~Acceptor();

    void start();

    void handleRead();
    void handleWrite();

    void setNewConnectionCallback(const NewConnectionCallback &cb); //设置新连接到来时的回调函数


};


#endif //MYWEBSERVER2_ACCEPTOR_H
