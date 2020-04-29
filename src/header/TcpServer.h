/// \file TcpServer.h
///
/// TcpServer的具体实现, 负责管理Acceptor和Manager, 前者负责建立连接, 后者负责每个单独线程里的连接管理
/// 利用模板, 不同的Manager实现不同的功能
///
/// \author wyw
/// \version 1.0
/// \date 2020/2/18.


#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H


#include <memory>
#include <unordered_map>
#include <functional>

#include "IHolder.h"
#include "Buffer.h"
#include "ConnectionManager.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"
#include "TcpConnection.h"

class Acceptor;
class EventLoop;
class TcpConnection;
class IHolder;
class EventLoopThreadPool;

template<typename Manager = ConnectionManager>
class TcpServer {
private:
    // 管理的Acceptor和TcpConnection
    using SPtrAcceptor = std::shared_ptr<Acceptor>;
    using SPtrConnection = std::shared_ptr<TcpConnection>;
    using SptrManager = std::shared_ptr<Manager>;

    // 用于TcpConnection的回调, 用户层面
    using ConnectionCallback = std::function<void(const SPtrConnection &)>;
    using MessageCallback = std::function<void(const SPtrConnection&, Buffer *)>;
    using WriteCompleteCallback = std::function<void(const SPtrConnection&)>;
private:
    EventLoop *mMainLoop;
    int mNumThreads;
    std::unique_ptr<EventLoopThreadPool> mUPtrThreadPool;

    std::unordered_map<EventLoop *, SptrManager> mConnectionManagerMap;
    SPtrAcceptor mPtrAcceptor; // 处理新连接
    // 用于TcpConnection的回调,  用户层面
    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;
public:
    explicit TcpServer(EventLoop *loop, int numThreads=4);
    ~TcpServer();
    /// TcpServer启动
    void start();

    /// 设置TcpConnection回调, 用户层面
    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);
private:
    /// 用于Acceptor的回调, Listen到结果执行该回调
    void newConnection(int sockFd); // 只能在在mMainLoop执行
    /// 用于TcpConnection的回调, TcpConnection连接断开时的回调函数, 删除TcpConnection
};


template<typename Manager>
TcpServer<Manager>::TcpServer(EventLoop *loop, int numThreads)
        : mMainLoop(loop),
          mNumThreads(numThreads),
          mUPtrThreadPool(new EventLoopThreadPool(mMainLoop,mNumThreads)),
          mPtrAcceptor(new Acceptor(mMainLoop)),
          mConnectionCallback(nullptr),
          mMessageCallback(nullptr),
          mWriteCompleteCallback(nullptr),
          mConnectionManagerMap()
{
    // 创建Acceptor
    //mPtrAcceptor = std::make_shared<Acceptor>(mMainLoop);
    // 设置Acceptor接到新连接时的回调
    mPtrAcceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));

}


template<typename Manager>
TcpServer<Manager>::~TcpServer()
{
    mMainLoop->assertInLoopThread();
    //TODO
}

template<typename Manager>
void TcpServer<Manager>::start()
{
    mUPtrThreadPool->start();
    for(int i=0;i<mNumThreads;i++){
        EventLoop *curLoop = mUPtrThreadPool->getNextLoop();
        //new Manager(curLoop);
        mConnectionManagerMap[curLoop] = std::shared_ptr<Manager>(new Manager(curLoop));
    }

    // Acceptor开始
    mPtrAcceptor->start();
}

template<typename Manager>
void TcpServer<Manager>::setConnectionCallback(const TcpServer::ConnectionCallback &cb)
{
    mConnectionCallback = cb;
}

template<typename Manager>
void TcpServer<Manager>::setMessageCallback(const TcpServer::MessageCallback &cb)
{
    mMessageCallback = cb;
}

template<typename Manager>
void TcpServer<Manager>::setWriteCompleteCallback(const TcpServer::WriteCompleteCallback& cb)
{
    mWriteCompleteCallback = cb;
}

template<typename Manager>
void TcpServer<Manager>::newConnection(int sockFd)
{
    mMainLoop->assertInLoopThread();
    EventLoop *loop = mUPtrThreadPool->getNextLoop();

    // 创建新连接的相关内容
    SPtrConnection newConnection = std::make_shared<TcpConnection>(loop, sockFd);
    // 放到管理map中
    if(mConnectionManagerMap.find(loop) != mConnectionManagerMap.end()){
        auto manager  = mConnectionManagerMap[loop];
        manager->addConnection(newConnection);
        // 设置Connection关闭时的回调, Connection关闭时让manager删除与它有关的内容
        newConnection->setCloseCallback(std::bind(&Manager::deleteConnection, manager,std::placeholders::_1));
    }

    // 设置TcpConnection回调, 用户层面
    // 如果每个Connection对应的有上下文信息，应该在manager里设置，不在这里设置
    if(mConnectionCallback != nullptr){
        mConnectionCallback(newConnection);
    }
    if(mMessageCallback != nullptr){
        newConnection->setMessageCallback(mMessageCallback);
    }
    if(mWriteCompleteCallback != nullptr){
        newConnection->setWriteCompleteCallback(mWriteCompleteCallback);
    }

    // 调用Connection建立连接时的相关函数
    loop->runInLoop(std::bind(&TcpConnection::start,newConnection));
}


#endif //MYWEBSERVER_TCPSERVER_H
