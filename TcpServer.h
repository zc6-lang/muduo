#pragma once

#include "noncopyable.h"
#include "Acceptor.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"

#include <functional>
#include <string>
#include <atomic>
#include <unordered_map>

//对外的服务器编程使用的类
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
        const InetAddress &listenAddr,
        const std::string &nameArg,
        Option option=kNoReusePort);
    ~TcpServer();

    //设置底层subloop的个数
    void setThreadNum(int numThreads);

    void setThreadInitcallback(const ThreadInitCallback &cb){ThreadInitCallback_=cb;}
    void setConnectionCallback(const ConnectionCallback &cb){ConnectionCallback_=cb;}
    void setMessageCallback(const MessageCallback &cb){MessageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb){WriteCompleteCallback_=cb;}

    //开启服务器监听
    void start();


private:
    void newConnection(int sockfd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;

    EventLoop *loop_;//mainloop
    
    const std::string ipPort_;
    const std::string name_;
    
    std::unique_ptr<Acceptor> acceptor_;//运行在mainloop,任务就是监听新连接事件
    
    std::shared_ptr<EventLoopThreadPool> threadPool_;//one loop one thread

    ConnectionCallback ConnectionCallback_;//有新连接时的回调
    MessageCallback MessageCallback_;//有读写消息时的回调
    WriteCompleteCallback WriteCompleteCallback_;//消息发送完成以后的回调
    ThreadInitCallback ThreadInitCallback_;//loop线程初始化的回调
    std::atomic_int started_;

    int nextConnId_;

    ConnectionMap connections_;//保存所有连接
};