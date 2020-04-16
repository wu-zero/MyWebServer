///// \file client.cpp
///// \brief
/////
///// Tcp客户端测试
/////
///// \author wyw
///// \version 1.0
///// \date 2020/3/12.



#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>

#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

#include "SocketUtils.h"

const int PORT = 55555;


int main()
{
    // 创建套接字
    int connectFd;
    if (-1 == (connectFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        std::cout << "Create Socket Error" << std::endl;
    }

    //向服务器（特定的IP和端口）发起请求
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);
    if (-1 == connect(connectFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) //客户端建立连接
    {
        std::cout << "Connect Error" << std::endl;
        exit(0);
    }
    std::cout << "Connect Success" << std::endl;
    bool run_flag = true;
    while (run_flag)
    {
        char buff[1024];
        memset(buff, 0, sizeof(buff));
        char Message[256];
        int mod = 0;
        std::cout << "chose mod: " << std::endl;
        std::cin >> mod;
        while(std::cin.fail())
        {
            std::string num_input_string;
            std::cin.clear();
            std::cin>>num_input_string;
            std::cout<<"输入的"<<num_input_string<<"不是数字，重新输入"<<std::endl;
        }

        std::cin.get();
        std::cout << "mod: " << mod <<std::endl;
        if (mod == 1)
        {
            std::cout << "Please input message:" << std::endl;
            std::cin.get(Message, 20);

            strcpy(buff, Message);
            int n = SocketUtils::writen(connectFd, buff, strlen(buff));//将数据发送给服务器端
            printf("Send %d byte to Server\n", n);
        } else if (mod == 2)
        {
            run_flag = false;
        }
    }
    shutdown(connectFd, SHUT_WR);
    close(connectFd);
    return 0;
}
