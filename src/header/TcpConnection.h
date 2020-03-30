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
class IUser;

class TcpConnection
{
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection> &)>;
private:
    EventLoop *mLoop; // 所属EventLoop
    int mSocketFd; // 负责连接的socket的文件描述符
    SPtrChannel mPtrChannel; //观察 负责连接的socket的文件描述符 的Channel
    Buffer mInBuffer; // 输入缓存
    Buffer mOutBuffer; // 输出缓存
    IUser *mPtrUser; // 用户

    CloseCallback mCloseCallback;
public:
    TcpConnection(EventLoop *eventLoop, int socketFd);
    ~TcpConnection();

    int getSocketFd() const{
        return mSocketFd;
    }

    void start(); // 建立连接时的操作：Channel、用户

    void send(const std::string &message);

    // 用于上层(管理TcpConnection的层)设置回调
    void setCloseCallback(const CloseCallback& cb);
    // 设置用户
    void setUser(IUser *ptrUser);
private:
    // 用于Channel的回调
    void handleRead();
    void handleWrite();
    void handleClose();

    // 执行完相关操作的用户回调
    void userConnectCallback();
    void userReadCallback(Buffer *pBuf);
    void userWriteCallback();
};


#endif //MYWEBSERVER_TCPCONNECTION_H
