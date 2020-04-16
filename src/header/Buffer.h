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
    /// 获取可取区域的起始地址
    const char* peek();
    /// 可取数据的大小
    int readableBytes();
    /// 缓冲区可取数据减少 len
    void retrieve(int len);
    /// 添加字符串数据到可取区
    void append(const std::string &data);
    /// 缓冲区可读区容量减少 len，并返回该部分的数据
    std::string retrieveAsString(size_t len);
    /// 将可读区域容量减少到 0，并返回该部分的数据
    std::string retrieveAllAsString();
};


#endif //MYWEBSERVER_BUFFER_H
