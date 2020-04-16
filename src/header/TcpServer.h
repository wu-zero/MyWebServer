/// \file TcpServer.h
///
/// TcpServer的具体实现, 负责管理Acceptor和Connection, 前者负责建立连接, 后者负责每个连接
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H


#include <memory>
#include <unordered_map>
#include <functional>
#include "IHolder.h"
#include "Buffer.h"

class Acceptor;
class EventLoop;
class TcpConnection;
class IHolder;

class TcpServer{
private:
    // 管理的Acceptor和TcpConnection
    using SPtrAcceptor = std::shared_ptr<Acceptor>;
    using SPtrConnection = std::shared_ptr<TcpConnection>;
    // 用于TcpConnection的回调, 用户层面
    using ConnectionCallback = std::function<void(const SPtrConnection &)>;
    using MessageCallback = std::function<void(const SPtrConnection&, Buffer *)>;
    using WriteCompleteCallback = std::function<void(const SPtrConnection&)>;
private:
    EventLoop *mEventLoop;
    SPtrAcceptor mPtrAcceptor; // 处理新连接
    std::unordered_map<int, SPtrConnection> mConnectionMap; // 所有建立的连接
    // 用于TcpConnection的回调,  用户层面
    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;
public:
    explicit TcpServer(EventLoop *loop);
    ~TcpServer();
    /// TcpServer启动
    void start();

    /// 设置TcpConnection回调, 用户层面
    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);
private:
    /// 用于Acceptor的回调, Listen到结果执行该回调
    void newConnection(int sockFd);
    /// 用于TcpConnection的回调, TcpConnection连接断开时的回调函数, 删除TcpConnection;
    void removeConnection(const SPtrConnection &connection);
};


#endif //MYWEBSERVER_TCPSERVER_H
