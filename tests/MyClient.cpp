///// \file client.cpp
///// \brief
/////
///// 客户端测试
/////
///// \author wyw
///// \version 1.0
///// \date 2020/4/5.


#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>


using namespace std;


#define IP_ADDRESS "127.0.0.1"
#define PORT  55555

int setSocketNonBlocking1(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}
int main(int argc,char *argv[])
{
    /// 设置特定的IP和端口
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    serverAddr.sin_port = htons(PORT);
    const char *p = " ";
    const char *p2 = " ";
    char buff[4096];
    int connectionFd;
    ///  开始测试
    /// 测试一：发送空request, 服务器返回400==============================================================================
    // 创建套接字
    if (-1 == (connectionFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        std::cout << "Create Socket Error" << std::endl;
    }
    // 连接
    if (connect(connectionFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0 )
    {
        setSocketNonBlocking1(connectionFd);
        cout << "=====================测试一=====================" << endl;
        ssize_t n = write(connectionFd, p, strlen(p));
        cout << "write... " << n << endl;
        sleep(1);
        n = read(connectionFd, buff, 4096);
        cout << "read..." << n << endl;
        cout<< buff <<endl;
    }
    else{
        perror("err1");
        exit(0);
    }
    sleep(1);
    close(connectionFd);

    /// 测试二：短链接, 服务器返回数据后，主动关闭=========================================================================
    p = "GET / HTTP/1.0\r\n"
        "Host: 127.0.0.1:55555\r\n"
        //"Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "DNT:1\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.163 Safari/537.36\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
        "Sec-Fetch-Site: none\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n\r\n";
    // 创建套接字
    if (-1 == (connectionFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        std::cout << "Create Socket Error" << std::endl;
    }
    // 连接
    if (connect(connectionFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
    {
        setSocketNonBlocking1(connectionFd);
        cout << "=====================测试二=====================" << endl;
        ssize_t n = write(connectionFd, p, strlen(p));
        cout << "write... " << n << endl;
        sleep(1);
        n = read(connectionFd, buff, 4096);
        cout << "read..." << n << endl;
        cout<< buff <<endl;
    }
    else
    {
        perror("err2");
        exit(0);
    }
    sleep(10);
    close(connectionFd);

    /// 测试二：长链接, 服务器返回数据后，不主动关闭========================================================================
    p = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:55555\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "DNT:1\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.163 Safari/537.36\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
        "Sec-Fetch-Site: none\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n\r\n";
    p2 = "GET /favicon.ico HTTP/1.1\r\n"
         "Host: 127.0.0.1:55555\r\n"
         "Connection: keep-alive\r\n"
         "Cache-Control: max-age=0\r\n"
         "DNT:1\r\n"
         "Upgrade-Insecure-Requests: 1\r\n"
         "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.163 Safari/537.36\r\n"
         "Sec-Fetch-Dest: document\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
         "Sec-Fetch-Site: none\r\n"
         "Sec-Fetch-Mode: navigate\r\n"
         "Sec-Fetch-User: ?1\r\n"
         "Accept-Encoding: gzip, deflate, br\r\n"
         "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n\r\n";
    // 创建套接字
    if (-1 == (connectionFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        std::cout << "Create Socket Error" << std::endl;
    }
    // 连接
    if (connect(connectionFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
    {
        setSocketNonBlocking1(connectionFd);
        cout << "=====================测试二=====================" << endl;
        // 第一次request
        ssize_t n = write(connectionFd, p, strlen(p));
        cout << "write... " << n << endl;
        sleep(1);
        n = read(connectionFd, buff, 4096);
        cout << "read..." << n << endl;
        cout<< buff <<endl;
        // 第二次request
        ssize_t n2 = write(connectionFd, p2, strlen(p2));
        cout << "write... " << n << endl;
        sleep(1);
        n = read(connectionFd, buff, 4096);
        cout << "read..." << n << endl;
        cout<< buff <<endl;
    }
    else
    {
        perror("err3");
        exit(0);
    }
    sleep(20);
    close(connectionFd);
}
