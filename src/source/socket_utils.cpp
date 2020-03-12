/// \file socket_utils.cpp
///
/// 与socket有关的工具
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "socket_utils.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <zconf.h>
#include <cstring>


const int MAX_LISTEN = 2048;
const int PORT = 55555;

int creatSocketAndListen(){
    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listenFd = 0;
    if((listenFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;
    // 设置服务器Ip和Port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)PORT);
    if(bind(listenFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    // 开始监听，最大等待队列长为LISTENQ
    if(listen(listenFd, MAX_LISTEN) == -1)
        return -1;

    // 无效监听描述符
    if(listenFd == -1)
    {
        close(listenFd);
        return -1;
    }
    return listenFd;
}

int setSocketNonBlock(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

int creatNewAccept(int listenFd){

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    int connFd = 0;
    //accept(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len)
    //accept4(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len,SOCK_NONBLOCK | SOCK_CLOEXEC)
    if((connFd = accept(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len)) < 0)
    {
        std::cout << "CreatNewAccept failed, connFd:" << connFd
                  << " errno:" << errno << std::endl;
    }
    else
    {
        std::cout << "CreatNewAccept from "
                  << "[" << inet_ntoa(client_addr.sin_addr)
                  << ":" << ntohs(client_addr.sin_port) << "]   "
                  << "New socket fd: " << connFd
                  << std::endl;
    }

    return connFd;
}


ssize_t readn(int fd, void *buff, size_t n){
    size_t numLeft = n;
    ssize_t numRead = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while (numLeft > 0)
    {
        if ((numRead = read(fd, ptr, numLeft)) < 0)
        {
            if (errno == EINTR)
                numRead = 0;
            else if (errno == EAGAIN) // 没有数据可读，稍后再试
            {
                return readSum;
            }
            else
            {
                return -1;
            }
        }
        else if (numRead == 0)
            break;
        readSum += numRead;
        numLeft -= numRead;
        ptr += numRead;
    }
}

ssize_t writen(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char *) buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN)
                {
                    return writeSum;
                } else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}