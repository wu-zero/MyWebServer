/// \file Buffer.h
///
/// 数据缓冲区, Tcp读到和要写的缓冲, 读到的数据存到这用多少取多少, 要写的数据存到这没写完继续写。
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.


#ifndef MYWEBSERVER_BUFFER_H
#define MYWEBSERVER_BUFFER_H

#include <string>

class Buffer
{
private:
    std::string mBuff;
public:
    Buffer();
    ~Buffer();
    const char* peek(); // 获取可读区域的起始地址
    int readableBytes(); // 可取数据的大小
    void retrieve(int len); // 缓冲区可读区容量减少 len
    void append(const std::string &data); // 添加字符串数据到可读区
    std::string retrieveAllAsString(); // 将可读区域容量减少到 0，并读出该部分的数据。
    std::string retrieveAsString(size_t len); // 缓冲区可读区容量减少 len，并读出该部分的数据。
};


#endif //MYWEBSERVER_BUFFER_H
