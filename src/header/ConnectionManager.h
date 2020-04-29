/// \file ConnectionManager.h
///
/// Manager基类, 只负责基本的Connection的管理了, 即：添加和删除
///
/// \author wyw
/// \version 1.0
/// \date 2020/4/28.


#ifndef MYWEBSERVER_CONNECTIONMANAGER_H
#define MYWEBSERVER_CONNECTIONMANAGER_H

#include <memory>
#include <unordered_map>
class EventLoop;
class TcpConnection;

class ConnectionManager {
public:
    using SPtrConnection = std::shared_ptr<TcpConnection>;
protected:
    EventLoop *mLoop;
private:
    std::unordered_map<int, SPtrConnection> mConnectionMap; // 所有建立的连接

public:
    explicit ConnectionManager(EventLoop *loop);
    ~ConnectionManager();
    virtual void addConnection(const SPtrConnection& sPtrConnection);
    virtual void addConnectionInLoop(const SPtrConnection& sPtrConnection);
    virtual void deleteConnection(const SPtrConnection& sPtrConnection);
    virtual void deleteConnectionInLoop(const SPtrConnection& sPtrConnection);
};


#endif //MYWEBSERVER_CONNECTIONMANAGER_H
