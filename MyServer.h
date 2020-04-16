/// \file MyServer.h
///
/// MyServer包含一个TcpServer, 每个TcpConnection通过weak_ptr与一个继承了IHolder类的HttpHandler互相绑定来实现Http功能。
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/5.


#ifndef MYWEBSERVER_MYSERVER_H
#define MYWEBSERVER_MYSERVER_H


#include <list>
#include "EventLoop.h"
#include "TcpServer.h"
#include "IHolder.h"
#include "HttpHandler.h"
#include "Timer.h"

class MyServer
{
private:
    using SPtrIContext = std::shared_ptr<IHolder>;
    using SPtrTcpConnection = std::shared_ptr<TcpConnection>;
    using SPtrHttpConnection = std::shared_ptr<HttpHandler>;

    using WPtrIContext = std::weak_ptr<IHolder>;
    using WPtrTcpConnection = std::weak_ptr<TcpConnection>;
    using WPtrHttpConnection = std::weak_ptr<HttpHandler>;

    // 长连接相关
    using TimerNode = std::pair<SPtrHttpConnection, Timer::TimeStampType >;
    using TimerNodeIter = std::list<TimerNode>::iterator;

    static const Timer::TimeStampType kKeepAliveTime;
    static const Timer::TimeStampType kExpireEventCircleInterval;
private:
    EventLoop *mPtrLoop;
    TimerManager mTimerManager;
    TcpServer mServer;

    std::list<TimerNode> mTimerNodeList;
    std::unordered_map<SPtrHttpConnection, TimerNodeIter> mTimerNodeMap;

public:
    explicit MyServer(EventLoop *pLoop);
    ~MyServer();
    void start();

    /// TcpConnection接口部分, 设置http层面的功能
    void onConnection(const SPtrTcpConnection& sPtrTcpConnection);
    void onMessage(const SPtrTcpConnection& sPtrTcpConnection, Buffer* pBuf);
    void onWriteComplete(const SPtrTcpConnection& sPtrTcpConnection);
    /// 长连接
    void updateKeepAlive(const SPtrHttpConnection& sPtrHttpConnection);
    void handleExpireEvent();
};


#endif //MYWEBSERVER_MYSERVER_H
