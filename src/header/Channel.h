/// \file Channel.h
///
/// 每个 Channel 对象负责一个socket文件描述符，
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
private:
    using Callback = std::function<void()>;

private:
    EventLoop *mLoop; // 所属的事件循环
    int mSocketFd; // 负责的socket文件描述符
    __uint32_t mEvents; // 告诉内核需要监听的事件类型, epoll_event.events, EPOLLIN、EPOLLOUT等宏的集合
    __uint32_t mRevents; // 目前活动事件, 由EventLoop负责设置

    Callback mReadHandler; // 读事件回调
    Callback mWriteHandler; // 写事件回调
    Callback mCloseHandler; // 关闭事件回调

    int mStatus;  // 现在的状态, 用于EPOLL_CTL_ADD或EPOLL_CTL_MOD选择

public:
    Channel(EventLoop *loop, int socketFd);
    ~Channel();
    void remove();
    // 负责的socket文件描述符
    int getSocketFd();
    void setSocketFd(int socketFd);

    // 设置监听事件
    void setEvents(__uint32_t event);
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    // 获得监听事件
    __uint32_t getEvents() const;
    bool isWriting();

    //设置活动事件类型
    void setRevents(__uint32_t revent);

    //处理回调
    void handleEvent();

    //设置回调函数
    void setReadHandler(const Callback &cb);
    void setWriteHandler(const Callback &cb);
    void setCloseHandler(const Callback &cb);

    //
    void setStatus(int status);
    int getStatus();

private:
    void update(); //更新需要监听的事件类型
};


#endif //MYWEBSERVER_CHANNEL_H
