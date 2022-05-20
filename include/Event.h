#ifndef EVENT_H
#define EVENT_H

#include<functional>
#include<chrono>
#include<memory>
#include<iostream>
#include"Base.h"
#include"Callbacks.h"

struct EventCompare;

//一个事件对应一个回调函数
class Event:noncopyable //继承noncopyable的原因？@wanghs
{
private:
    /* data */
    Time m_time;   //事件的完成时刻@wanghs
    ITimeCallback m_TimeCallback; //事件完成时的回调函数@wanghs
public:
    Event();
    ~Event();
    Event(Time time);
    void setITimeCallback(ITimeCallback callback);
    ITimeCallback getITimeCallBack();
    void setTime(Time time);
    Time getTime()const;
    void HandleEvent();
};



/*这个是仿函数，其事件按照降序排列，是否有问题？@wanghs*/
struct EventCompare{
    bool operator()(std::shared_ptr<Event> rhs,std::shared_ptr<Event> rhl){
        return rhs->getTime()>rhl->getTime();
    }
};

#endif