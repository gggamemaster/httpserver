//
//
//
//
//IO复用流程的抽象，等待事件，处理事件，执行其他任务

#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "Poller.h"
#include "Channel.h"

class EventLoop /*nocopyable*/
{
public:
    //任务类型
    typedef std::function<void()> Functor;

    //事件列表类型
    typedef std::vector<Channel*> ChannelList;  //eventloop管理的channel

    EventLoop(/* args */);
    ~EventLoop();

    //执行事件循环
    void loop();

    //添加通道事件
    void AddChannelToPoller(Channel *pchannel)
    {
        poller_.AddChannel(pchannel);
    }

    //移除通道事件
    void RemoveChannelToPoller(Channel *pchannel)
    {
        poller_.RemoveChannel(pchannel);
    }

    //修改事件
    void UpdateChannelToPoller(Channel *pchannel)//更新通道事件
    {
        poller_.UpdateChannel(pchannel);
    }

    //退出事件循环
    void Quit()
    {
        quit_ = true;
    }

    //获取loop所在线程id
    std::thread::id GetThreadId() const
    {
        return tid_;
    }

    //唤醒loop
    void WakeUp();

    //唤醒loop后的读回调，即把wakeupfd中的消息读走
    void HandleRead();

    //唤醒loop后的错误处理回调
    void HandleError();

    //向任务队列添加任务
    void AddTask(Functor functor)
    {
        {
            std::lock_guard <std::mutex> lock(mutex_);
            //std::cout << "push_back done" << std::endl;
            functorlist_.push_back(functor);

        }
        //std::cout << "WakeUp" << std::endl;
        WakeUp();//跨线程唤醒，worker线程唤醒IO线程，为啥要唤醒，因为有些任务必须交给eventloop中的循环中去处理,起到一个延后执行的作用。
    }

    //执行任务队列的任务
    void ExecuteTask()
    {
        // std::lock_guard <std::mutex> lock(mutex_);
        // for(Functor &functor : functorlist_)
        // {
        //     functor();//在加锁后执行任务，调用sendinloop，再调用close，执行添加任务，这样functorlist_就会修改
        // }
        // functorlist_.clear();

        std::vector<Functor> functorlist;
        {
            std::lock_guard <std::mutex> lock(mutex_);
            functorlist.swap(functorlist_);
        }
        for(Functor &functor : functorlist)
        {
            functor();//执行任务,有些任务比如tcpconnection的移除，因为生存周期的问题，要交给eventloop处理
        }
        functorlist.clear();
    }

private:
    //任务列表
    std::vector<Functor> functorlist_;

    //所有事件，暂时不用
    ChannelList channellist_;

    //活跃事件
    ChannelList activechannellist_;

    //epoll操作封装
    Poller poller_;

    //运行状态
    bool quit_;

    //loop所在的线程id
    std::thread::id tid_;

    //保护任务列表的互斥量
    std::mutex mutex_;

    //跨线程唤醒fd
    int wakeupfd_;

    //用于唤醒当前loop的事件
    Channel wakeupchannel_;
};


#endif
