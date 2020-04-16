/// \file Epoll.h
///
/// Epoll通过Channel监听Channel负责的文件描述符, 不负责管理Channel
/// 通过updateChannel添加监听fd, 通过removeChannel删除监听fd
/// 只负责与系统相关的epoll部分, 不管理相关对象
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.


#ifndef MYWEBSERVER_EPOLL_H
#define MYWEBSERVER_EPOLL_H

#include <vector>
#include <memory>
#include <sys/epoll.h>


class Channel;


class Epoll
{
private:
    //using SPtrChannel = std::shared_ptr<Channel>;
    using ChannelVector = std::vector<Channel *>; // Channel列表
private:
    int mEpollFd; // epoll 文件描述符
    static const int kMaxEventListSize = 16;
    struct epoll_event mEvents[kMaxEventListSize]{};
public:
    Epoll();
    ~Epoll();

    void poll(ChannelVector &channelVector); // 监听文件描述符

    void updateChannel(Channel *channel); // 添加监听
    void removeChannel(Channel *channel); // 删除监听
};


#endif //MYWEBSERVER_EPOLL_H
