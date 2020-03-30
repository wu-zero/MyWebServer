/// \file socket_utils.h
///
/// 与socket有关的工具
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_SOCKET_UTILS_H
#define MYWEBSERVER_SOCKET_UTILS_H


#include<sys/types.h>


int creatSocketAndListen();
int setSocketNonBlock(int fd);
int creatNewAccept(int listenfd);

ssize_t readn(int fd, char *buff, size_t n);
ssize_t writen(int fd, const char *buff, size_t n);


#endif //MYWEBSERVER_SOCKET_UTILS_H
