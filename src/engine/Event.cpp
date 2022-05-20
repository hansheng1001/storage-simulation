#include"Event.h"

Event::Event(Time time):
    m_time(time)
{
    //func=std::move(std::bind(&HandleCallBack,this));
}
Event::Event(){
}
Event::~Event(){
}

void Event::setITimeCallback(ITimeCallback callback){
    m_TimeCallback=std::move(callback);
}

ITimeCallback Event::getITimeCallBack(){
    return std::move(m_TimeCallback);
}

void Event::setTime(Time time){
    m_time=time;
}

Time Event::getTime()const{
    return m_time;
}

void Event::HandleEvent(){
    m_TimeCallback();
}