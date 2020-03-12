/// \file client.cpp
/// \brief 
///
/// A detailed file description.
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/12.



#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>

#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

#include "socket_utils.h"

const int  PORT=55555;


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main()
{
    int connectFd;

    struct sockaddr_in serverAddr;
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serverAddr.sin_port=htons(PORT);

    if(-1==(connectFd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) //客户端 创建
    {
        std::cout << "Create Socket Error" << std::endl;
    }
    if(-1==connect(connectFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) //客户端建立连接
    {
        std::cout << "Connect Error" << std::endl;
        exit(0);
    }
    std::cout << "Connect Success" << std::endl;
    while(true)
    {
        char buff[1024];
        memset(buff, 0, sizeof(buff));
        char Message[256];
        std::cout << "Please input message:" << std::endl;
        std::cin.get(Message, 20);

        strcpy(buff, Message);
        int n = writen(connectFd, buff, strlen(buff));//将数据发送给服务器端
        printf("Send %d byte to Server\n",n);

    }
    return 0;
}
#pragma clang diagnostic pop
