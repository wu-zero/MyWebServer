/// \file TcpServer.h
///
/// Server的具体实现，epoll监听一个socket，
/// 对于Client请求，用Acceptor建立新的socket连接，epoll中增加新的监听socket
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER2_TCPSERVER_H
#define MYWEBSERVER2_TCPSERVER_H


#include <sys/epoll.h>
#include <map>


#include "TcpConnection.h"
#include "Acceptor.h"


class TcpServer{
private:
    enum{
        MAX_EVENTS = 10,
        MAX_LISTEN = 100,
    };
    using SPtrAcceptor = std::shared_ptr<Acceptor>;
    using SPtrConnection = std::shared_ptr<TcpConnection>;
private:
    int mEpollFd; // epoll文件描述符
    struct epoll_event mEvents[MAX_EVENTS]; // epoll_wait从内核得到事件的集合(临时文件)
    std::map<int, SPtrConnection> mConnections; // 所有建立的连接
    SPtrAcceptor mPtrAcceptor; //处理新连接

public:
    TcpServer();
    ~TcpServer();
    void start();
    void newConnection(int sockfd); //新连接到来时的回调函数
};


#endif //MYWEBSERVER2_TCPSERVER_H
