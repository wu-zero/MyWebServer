/// \file Channel.h
///
/// 每个 Channel 对象负责一个socket文件描述符，
/// 需要设置要监听的事件类型以及对应的回调函数，
/// 执行回调时，先指明监听到的事件类型，再执行回调。
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER2_CHANNEL_H
#define MYWEBSERVER2_CHANNEL_H


#include <functional>


class Channel{
private:
    using Callback = std::function<void()>;
private:
    int mEpollFd; // 对应的epoll文件描述符
    int mSocketFd; // 对应的socket文件描述符
    __uint32_t mEvents; // epoll_event.events, EPOLLIN、EPOLLOUT等宏的集合，告诉内核需要监听的事件类型
    __uint32_t mRevents; // 发生的事件类型

    Callback mReadHandler;
    Callback mWriteHandler;
    Callback mCloseHandler;

public:
    explicit Channel(int epollFd);
    ~Channel();

    int getSocketFd();
    void setSocketFd(int socketFd);

    void setReadHandler(const Callback &cb); //设置回调函数
    void setWriteHandler(const Callback &cb);
    void setCloseHandler(const Callback &cb);

    void handleEvent(); //处理回调

    void setEvents(__uint32_t event);
    void setRevents(__uint32_t revent); //设置发生的事件类型

    void enableReading();
    void enableWriting();
private:
    void update(); //更新需要监听的事件类型
};


#endif //MYWEBSERVER2_CHANNEL_H
