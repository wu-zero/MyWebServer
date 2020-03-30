/// \file IUser.h
///
/// ClassB detailed file description.
///
/// \author wyw
/// \version 1.0
/// \date 2020/3/16.


#ifndef MYWEBSERVER_IUSER_H
#define MYWEBSERVER_IUSER_H

#include "TcpConnection.h"
class TcpConnection;

class IUser{
public:
    virtual void onConnection(TcpConnection *pCon) = 0;
    virtual void onMessage(TcpConnection* pCon, Buffer* pBuf) = 0;
    virtual void onWriteComplete(TcpConnection* pCon) = 0;
};


#endif //MYWEBSERVER_IUSER_H
