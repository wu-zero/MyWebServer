/// \file socket_utils.h
///
/// 与socket有关的工具
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_SOCKETUTILS_H
#define MYWEBSERVER_SOCKETUTILS_H


#include<sys/types.h>
#include <string>

namespace SocketUtils{
    /// 创建server的Socket并绑定端口
    int creatSocketAndListen(int port);
    /// 创建新的连接
    int creatNewAccept(int listenFd);
    /// 关闭fd
    void close(int sockFd);
    /// shutdown socketFd的写
    void shutdownWrite(int sockFd);
    /// 读数据
    ssize_t readn(int sockFd, const char *buff, size_t buffNum);
    /// 写数据
    ssize_t writen(int sockFd, const char *buff, size_t buffNum);
}


#endif //MYWEBSERVER_SOCKETUTILS_H
