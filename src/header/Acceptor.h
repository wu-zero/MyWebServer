/// \file Acceptor.h
///
/// 负责处理新连接，监听到有Read，就执行Read回调创建一个新的Accept，
/// 同时执行NewConnection回调，让Server执行相应操作（创建TcpConnection对象）
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_ACCEPTOR_H
#define MYWEBSERVER_ACCEPTOR_H

#include <memory>
#include <functional>

class EventLoop;
class Channel;

class Acceptor
{
private:
    using SPtrChannel = std::shared_ptr<Channel>;
    using NewConnectionCallback = std::function<void(int newConnectFd)>;
private:
    EventLoop *mPtrLoop; // 所属EventLoop
    int mListenFd; // (服务器监听的socket)的文件描述符
    SPtrChannel mPtrAcceptChannel; // 观察(服务器监听的socket)的Channel

    NewConnectionCallback mNewConnectionCallback; // 新连接到来时的回调函数
public:
    explicit Acceptor(EventLoop *loop);
    ~Acceptor();
    /// 开始监听server
    void start();
    /// 用于上层(管理Acceptor的层)，设置新连接到来时的回调函数
    void setNewConnectionCallback(const NewConnectionCallback &cb);
private:
    /// 用于Channel的回调
    void handleRead();
};


#endif //MYWEBSERVER_ACCEPTOR_H
