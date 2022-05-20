#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include<queue>
#include<memory>
#include<cctype>
#include<chrono>
#include<mutex>
#include<condition_variable>
#include<vector>

#include"Event.h"
#include"Callbacks.h"

class EventLoop
{
    typedef std::priority_queue<std::shared_ptr<Event>,std::vector<std::shared_ptr<Event>>,EventCompare> EventHeap;

    static EventLoop instance_;

private:
    /* data */
    EventHeap m_EventHeap;
    Time m_CurrentTime;
    std::vector<CallBack> Functor;
private: 
    void HandleEvent();
    EventLoop();
public:
    Time GetCurrentTime();
    void addCallBack(CallBack);
    ~EventLoop();
    void ChooseEventToStart();
    void AddEvent(std::shared_ptr<Event> event);
    void HandleCallBack(); //执行存放在Functor中的回调函数@wanghs
    void callAfter(Time time, CallBack callback);

    static EventLoop& getInstance();
};

#endif
