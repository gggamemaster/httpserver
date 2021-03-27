//
//
//
//
//
#include "EventLoop.h"
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>

//参照muduo，实现跨线程唤醒
int CreateEventFd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        std::cout << "Failed in eventfd" << std::endl;
        exit(1);
    }
    return evtfd;
}

EventLoop::EventLoop(/* args */)
    : functorlist_(),
    channellist_(),
    activechannellist_(),
    poller_(),
    quit_(true),
    tid_(std::this_thread::get_id()),
    mutex_(),
    wakeupfd_(CreateEventFd()),
    wakeupchannel_()
{
    wakeupchannel_.SetFd(wakeupfd_);
    wakeupchannel_.SetEvents(EPOLLIN | EPOLLET);//设置为et模式
    wakeupchannel_.SetReadHandle(std::bind(&EventLoop::HandleRead, this));//设置wekechanel的回调函数
    wakeupchannel_.SetErrorHandle(std::bind(&EventLoop::HandleError, this));
    AddChannelToPoller(&wakeupchannel_);//注册关注wakeupchannel
}

EventLoop::~EventLoop()
{
    close(wakeupfd_);
}

void EventLoop::WakeUp()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_, (char*)(&one), sizeof one);//往wakeupfd里发一个消息
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof one);//从wakeupfd读出一个消息q
}

void EventLoop::HandleError()
{
    ;
}

void EventLoop::loop()
{
    quit_ = false;
    while(!quit_)
    {
        poller_.poll(activechannellist_);
        //std::cout << "server HandleEvent" << std::endl;
        for(Channel *pchannel : activechannellist_)
        {
            pchannel->HandleEvent();//处理事件
        }
        activechannellist_.clear();
        ExecuteTask();
    }
}
