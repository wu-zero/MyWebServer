/// \file socket_utils.h
///
/// 与socket有关的工具
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER2_SOCKET_UTILS_H
#define MYWEBSERVER2_SOCKET_UTILS_H


#include<sys/types.h>


int creatSocketAndListen();
int setSocketNonBlock(int fd);
int creatNewAccept(int listenfd);

ssize_t readn(int fd, void *buff, size_t n);
ssize_t writen(int fd, void *buff, size_t n);


#endif //MYWEBSERVER2_SOCKET_UTILS_H
