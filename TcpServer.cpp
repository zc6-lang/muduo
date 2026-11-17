#include "TcpServer.h"
#include "Logger.h"

EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if(loop==nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null! \n",__FILE__,__FUNCTION__,__LINE__);

    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,const InetAddress &listenAddr,const std::string &nameArg,Option option)
        :loop_(CheckLoopNotNull(loop))
        ,ipPort_(listenAddr.toIpPort())
        ,name_(nameArg)
        ,acceptor_(new Acceptor(loop,listenAddr,option==kReusePort))
        ,threadPool_(new EventLoopThreadPool(loop,name_))
        ,ConnectionCallback_()
        ,MessageCallback_()
        ,nextConnId_(1)
{
    //当有新用户连接时，会执行TcpServer::newConnectino回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}

 //开启服务器监听
void TcpServer::start()
{
    if(started_++==0)
    {
        threadPool_->start(ThreadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

      //设置底层subloop的个数
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}