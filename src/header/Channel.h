/// \file Channel.h
///
/// 每个 Channel 对象负责一个文件描述符，
/// 需要设置要监听的事件类型以及对应的回调函数，
/// 执行回调时，先指明监听到的事件类型，再执行回调。
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_CHANNEL_H
#define MYWEBSERVER_CHANNEL_H


#include <functional>


class EventLoop;


class Channel
{
public:
    enum ChannelStatus{kChannelStatuesNew , kChannelStatuesAdded, kChannelStatuesDeleted}; // 现在的状态, 用于EPOLL_CTL_ADD或EPOLL_CTL_MOD选择
private:
    using Callback = std::function<void()>;
private:
    EventLoop *mLoop; // 所属的事件循环
    int mFd; // 负责的文件描述符(socketfd, timefd, eventfd)
    __uint32_t mEvents; // 告诉内核需要监听的事件类型, epoll_event.events, EPOLLIN、EPOLLOUT等宏的集合
    __uint32_t mRevents; // 目前活动事件, 由EventLoop负责设置

    Callback mReadHandler; // 读事件回调
    Callback mWriteHandler; // 写事件回调
    Callback mCloseHandler; // 关闭事件回调
    ChannelStatus mStatus; // 负责的fd在epoll中的状态

public:
    Channel(EventLoop *loop, int fileDescriptor);
    ~Channel();
    void remove();
    /// 负责的文件描述符
    int getFd();
    void setFd(int fileDescriptor);
    /// 设置监听事件
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();
    /// 获得监听事件
    __uint32_t getEvents() const;
    bool isWriting();
    bool isReading();
    /// 设置活动事件类型
    void setRevents(__uint32_t revent);
    /// 处理回调
    void handleEvent();
    /// 设置回调函数
    void setReadHandler(Callback cb);
    void setWriteHandler(Callback cb);
    void setCloseHandler(Callback cb);
    /// Channel负责的fd在epoll中的状态
    void setStatus(ChannelStatus status);
    ChannelStatus getStatus();
private:
    /// epoll里更新需要监听的事件类型
    void update();
};


#endif //MYWEBSERVER_CHANNEL_H
