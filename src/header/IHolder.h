/// \file IHolder.h
///
/// 每个TcpConnection与一个IHolder用weak_ptr关联, 用于实现http层的功能
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/3.


#ifndef MYWEBSERVER_IHOLDER_H
#define MYWEBSERVER_IHOLDER_H
class IHolder{
public:
    IHolder() = default;
    virtual ~IHolder() = default;
};

#endif //MYWEBSERVER_IHOLDER_H
