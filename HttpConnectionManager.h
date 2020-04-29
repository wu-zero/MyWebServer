/// \file HttpConnectionManager.h
///
/// 每个SubReactor里有一个HttpConnectionManager, 管理TcpConnection和HttpHandler
/// 每个TcpConnection通过weak_ptr与一个继承了IHolder类的HttpHandler互相绑定来实现Http功能。
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/29.


#ifndef MYWEBSERVER_HTTPCONNECTIONMANAGER_H
#define MYWEBSERVER_HTTPCONNECTIONMANAGER_H

#include "ConnectionManager.h"

#include <list>

#include "HttpHandler.h"
#include "Timer.h"

class HttpConnectionManager: public ConnectionManager{
public:
private:
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
    TimerManager mTimerManager;

    std::list<TimerNode> mTimerNodeList;
    std::unordered_map<SPtrHttpConnection, TimerNodeIter> mTimerNodeMap;

public:
    explicit HttpConnectionManager(EventLoop *loop);
    ~HttpConnectionManager();

    void addConnection(const SPtrConnection& sPtrConnection) override ;
    void addConnectionInLoop(const SPtrConnection& sPtrTcpConnection) override ;
    void deleteConnection(const SPtrConnection& sPtrConnection) override ;
    void deleteConnectionInLoop(const SPtrConnection& sPtrConnection) override ;

    /// 设置http层面的功能
    void onMessage(const SPtrTcpConnection& sPtrTcpConnection, Buffer* pBuf);
    void onWriteComplete(const SPtrTcpConnection& sPtrTcpConnection);
    /// 长连接
    void updateKeepAlive(const SPtrHttpConnection& sPtrHttpConnection);
    void handleExpireEvent();

};


#endif //MYWEBSERVER_HTTPCONNECTIONMANAGER_H
