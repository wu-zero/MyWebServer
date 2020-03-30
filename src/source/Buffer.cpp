/// \file Buffer.cpp
///
/// 数据缓冲区, Tcp读到和要写的缓冲, 读到的数据存到这用多少取多少, 要写的数据存到这没写完继续写。
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/15.


#include "Buffer.h"
#include <assert.h>

Buffer::Buffer()
:mBuff()
{
}

Buffer::~Buffer()
{
}

const char *Buffer::peek()
{
    return mBuff.c_str();
}

int Buffer::readableBytes()
{
    return static_cast<int>(mBuff.size());
}

void Buffer::retrieve(int len)
{
    assert(len <= readableBytes());
    mBuff = mBuff.substr(len, mBuff.size());
}

void Buffer::append(const std::string &data)
{
    mBuff.append(data);
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    std::string result(peek(),len);
    retrieve(len);
    return result;
}

