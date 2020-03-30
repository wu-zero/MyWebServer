/// \file TcpServer.h
///
/// TcpServer的具体实现, 负责管理Acceptor和Connection, 前者负责建立连接, 后者负责每个连接
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H


#include <map>
#include <memory>

class Acceptor;
class EventLoop;
class TcpConnection;
class IUser;


class TcpServer{
private:
    using SPtrAcceptor = std::shared_ptr<Acceptor>;
    using SPtrConnection = std::shared_ptr<TcpConnection>;
private:
    EventLoop *mEventLoop;
    SPtrAcceptor mPtrAcceptor; //处理新连接
    std::map<int, SPtrConnection> mConnectionMap; // 所有建立的连接
    IUser *mPtrIUser; // Connection的用户层
public:
    explicit TcpServer(EventLoop *loop);
    ~TcpServer();
    // TcpServer启动
    void start();
    //新连接到来时的回调函数, for Acceptor
    void newConnection(int sockFd);
    //连接断开时的回调函数, for Connection
    void removeConnection(const SPtrConnection &connection);
    //设置User接口
    void setUser(IUser *ptrIUser);
};


#endif //MYWEBSERVER_TCPSERVER_H
