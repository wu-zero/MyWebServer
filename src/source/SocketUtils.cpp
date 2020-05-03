/// \file socket_utils.cpp
///
/// 与socket有关的工具
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#include "SocketUtils.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <zconf.h>
#include <cstring>


const int MAX_LISTEN = 2048;
const int PORT = 55555;


//int setSocketNonBlock(int fd){
//    int flag = fcntl(fd, F_GETFL, 0);
//    if(flag == -1)
//        return -1;
//
//    flag |= O_NONBLOCK;
//    if(fcntl(fd, F_SETFL, flag) == -1)
//        return -1;
//    return 0;
//}

int SocketUtils::creatNewAccept(int listenFd){
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    int connFd = 0;
    //accept(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len)
    //accept4(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len,SOCK_NONBLOCK | SOCK_CLOEXEC)
    if((connFd = accept4(listenFd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len,SOCK_NONBLOCK | SOCK_CLOEXEC)) < 0)
    {
        perror("SocketUtils::creatNewAccept()");
    }
    else
    {
//        std::cout << "CreatNewAccept from "
//                  << "[" << inet_ntoa(client_addr.sin_addr)
//                  << ":" << ntohs(client_addr.sin_port) << "]   "
//                  << "New socket fd: " << connFd
//                  << std::endl;
    }

    return connFd;
}

int SocketUtils::creatSocketAndListen(int port) {
    if(port < 0 || port > 65535){
        return -1;
    }

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listenFd = -1;
    if((listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) == -1){
        perror("SocketUtils::creatSocketAndListen(): creat error");
        return -1;
    }

    // 设置服务器Ip和Port, 绑定
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listenFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("SocketUtils::creatSocketAndListen(): bind error");
        return -1;
    }

    // 开始监听，最大等待队列长为LISTENQ
    if(listen(listenFd, MAX_LISTEN) == -1){
        perror("SocketUtils::creatSocketAndListen(): listen error");
        return -1;
    }

    return listenFd;
}

void SocketUtils::close(int sockFd) {
    if(::close(sockFd) < 0){
        perror("SocketUtils::close");
    }
}

void SocketUtils::shutdownWrite(int sockFd) {
    if(::shutdown(sockFd, SHUT_WR) < 0){
        perror("SocketUtils::shutdownWrite");
    }
}

ssize_t SocketUtils::readn(int sockFd, const char *buff, size_t buffNum)
{
    size_t numLeft = buffNum;
    ssize_t numRead = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while (numLeft > 0)
    {
        if ((numRead = read(sockFd, ptr, numLeft)) < 0)
        {
            if (errno == EINTR)
                numRead = 0;
            else if (errno == EAGAIN) // 没有数据可读，稍后再试
            {
                return readSum;
            }
            else
            {
                perror("SocketUtils::readn");
                return -1;
            }
        }
        else if (numRead == 0)
            break;
        readSum += numRead;
        numLeft -= numRead;
        ptr += numRead;
    }
    return readSum;
}

ssize_t SocketUtils::writen(int sockFd, const char *buff, size_t buffNum){
    size_t leftNum = buffNum;
    ssize_t writtenNum = 0;
    ssize_t writeSum = 0;
    char *ptr = (char *)buff;
    while (leftNum > 0)
    {
        if ((writtenNum = write(sockFd, ptr, leftNum)) <= 0)
        {
            if (writtenNum < 0)
            {
                if (errno == EINTR)
                {
                    writtenNum = 0;
                    continue;
                } else if (errno == EAGAIN)
                {
                    return writeSum;
                } else{
                    perror("SocketUtils::writen");
                    return -1;
                }
            }
        }
        writeSum += writtenNum;
        leftNum -= writtenNum;
        ptr += writtenNum;
    }
    return writeSum;
}


