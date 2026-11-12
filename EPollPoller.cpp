#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>

const int kNew=-1;//channel未添加到poller中
const int kAdded=1;//channel已添加到poller中
const int kDeleted=2;//channel从poller中删除

EPollPoller::EPollPoller(EventLoop *loop)
            : Poller(loop)
            , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
            , events_(kInitEventListSize)
{
    if(epollfd_<0)
    {
        LOG_FATAL("epoll_create error:%d \n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}


Timestamp EPollPoller::poll(int timeoutMs,ChannelList *activeChannels)
{
    LOG_INFO("func=%s => fd total count:%d\n",channels_.size());

    int numEvent = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvent>0)
    {
        LOG_INFO("%d events happened \n",numEvent);
        fillActiveChannels(numEvent,activeChannels);

        if(numEvent == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvent==0)
    {
        LOG_DEBUG("%s timeout! \n",__FUNCTION__);
    }
    else
    {
        if(saveErrno!=EINTR)
        {
            errno=saveErrno;
            LOG_ERROR("EPollPoller::poll() err!\n");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int idx=channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(),idx);

    if( idx==kNew || idx=kDeleted)
    {
        if(idx==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        int fd=channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
    
}

//从poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd=channel->fd();
    channels_.erase(fd);

     LOG_INFO("func=%s => fd=%d \n",__FUNCTION__,channel->fd());
    
    int index=channel->index();
    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);

}

 //更新channel通道  
void EPollPoller::update(int operation,Channel *channel)
{
    epoll_event event;
    memset(&event,0,sizeof event);

    int fd=channel->fd();
    event.events=channel->events();
    event.data.fd=fd;
    event.data.ptr=channel;

    if(::epoll_ctl(epollfd_,operation,fd,&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl_del error:%d\n",errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl_add/mod error:%d\n",errno);
        }
    }
}

 //填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels)
{
    for(int i=0;i<numEvents;++i)
    {
        Channel *channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);//EventLoop就拿到了它的Poller给它返回的所有发生事件的channe列表了
    }
}