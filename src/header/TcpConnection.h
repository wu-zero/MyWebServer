/// \file TcpConnection.h
///
/// 每个新连接对应一个TcpConnection对象, 回调执行socket的相应读写事件, 通过EventLoop异步执行相应操作
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_TCPCONNECTION_H
#define MYWEBSERVER_TCPCONNECTION_H


#include <memory>
#include <functional>
#include "Buffer.h"

class Channel;
class EventLoop;
class IHolder;
class Buffer;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    // Tcp连接状态：连接、等待关闭(写完数据就关闭)、已关闭
    enum TcpConnectionState{kStateConnected, kStateDisconnecting, kStateDisconnected};
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using SPtrTcpConnection = std::shared_ptr<TcpConnection>;

    // TcpConnection删除时, TcpServer回调
    using CloseCallback = std::function<void(const SPtrTcpConnection &)>;

    // TcpConnection应用层持有者, HttpConnection
    using WPtrIHolder = std::weak_ptr<IHolder>;
    // TcpConnection应用层回调
    using ConnectionCallback = std::function<void(const SPtrTcpConnection &)>;
    using MessageCallback = std::function<void(const SPtrTcpConnection &, Buffer *)>;
    using WriteCompleteCallback =  std::function<void(const SPtrTcpConnection&)>;
private:
    EventLoop *mLoop; // 所属EventLoop
    int mSocketFd; // 负责连接的socket的文件描述符
    SPtrChannel mPtrChannel; // 观察(负责连接的socket的文件描述符)的Channel
    Buffer mInBuffer; // 输入缓存
    Buffer mOutBuffer; // 输出缓存
    TcpConnectionState mTcpConnectionState; // TcpConnection连接状态

    // TcpConnection删除时, TcpServer回调
    CloseCallback mCloseCallback;

    // TcpConnection应用层Holder和回调
    WPtrIHolder mWPtrContext;
    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;
public:
    TcpConnection(EventLoop *eventLoop, int socketFd);
    ~TcpConnection();
    /// TcpConnection对应的SocketFd
    int getSocketFd() const;
    /// TcpConnection连接状态
    void setState(TcpConnectionState state);
    void setStateInLoop(TcpConnectionState state);
    TcpConnectionState getState();
    /// TcpConnection的开始、shutdown和关闭
    void start(); // 建立连接时的操作：Channel、http
    void startInLoop();
    void shutdown();
    void shutdownInLoop();
    void close();
    void closeInLoop();

    /// TcpConnection删除时, TcpServer回调
    void setCloseCallback(const CloseCallback &cb);

    /// 设置TcpConnection应用层持有者
    void setHolder(WPtrIHolder wPtrIHolder);
    WPtrIHolder getHolder();
    /// TcpConnection应用层Holder和回调
    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setWriteCompleteCallback(const WriteCompleteCallback &cb);
    /// TcpConnection应用层发送消息
    void send(const std::string &message);
    void sendInLoop(const std::string &message);
private:
    /// 用于Channel的回调
    void handleRead();
    void handleWrite();
    void handleClose();
};


#endif //MYWEBSERVER_TCPCONNECTION_H
