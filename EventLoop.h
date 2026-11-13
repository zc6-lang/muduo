#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>


class Channel;
class Poller;

//时间循环类    主要包含两个大模块  Channel Poller(epoll的抽象)
class EventLoop:noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void quit();//退出事件循环
    Timestamp pollReturnTime() const {return pollReturnTime_;}

    void runInLoop(Functor cb);//在当前loop中执行
    void queueInLoop(Functor cb);//把cb放入队列中，唤醒loop所在的线程执行cb

    void wakeup();//用来唤醒loop所在的线程

    void updateChannel(Channel *channel);//eventloop的方法，poller的方法
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}//判断evetnloop对象是否在直接的线程里面

    void loop();//开启时间循环
private:
    void handleRead();//wake up
    void doPendingFunctors();//执行回调


    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  //原子操作，通过CAS实现的
    std::atomic_bool quit_;     //标志退出loop循环
    std::atomic_bool callingPendingFunctors_;//标识当前loop是否有需要执行的回调操作
    const pid_t threadId_;      //记录当前loop所在线程的id

    Timestamp pollReturnTime_;  //poller返回发生事件的channel的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;//主要作用：当mainLoop获取一个新用户的channel，通过轮询算法一个subLoop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::mutex mutex_;//用来保护vector的线程安全操作
    std::vector<Functor> pendingFunctors_;//存储loop需要执行的所有的回调操作

};