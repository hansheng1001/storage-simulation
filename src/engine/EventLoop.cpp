#include "EventLoop.h"

//测试
#include <iostream>

#include <mutex>

EventLoop EventLoop::instance_;

EventLoop::EventLoop() : m_CurrentTime(0) {}

EventLoop::~EventLoop() {}

int  debug = 0;

/*
功能：1、找出heap顶端的event，根据event的完成时间更新eventloop的时间
2、执行event中的回调函数
*/
void EventLoop::HandleEvent() {
    // if(debug==36){
    //     std::cout << "wocao" << std::endl;
    // }

    if (m_EventHeap.empty()) {
        return;
    }
	
    auto event = m_EventHeap.top();
    m_EventHeap.pop();

    m_CurrentTime = event->getTime();
    event->HandleEvent();
}

Time EventLoop::GetCurrentTime() {
    return m_CurrentTime;
}

void EventLoop::AddEvent(std::shared_ptr<Event> event) {
    m_EventHeap.push(event);
}

void EventLoop::ChooseEventToStart() {
    //在堆中找到与最小事件相等的事件

    while (!m_EventHeap.empty()) {  //事件为空
        if(m_CurrentTime>=UINT64_MAX-5000000000000ul*5){
            exit(0);
        }

        this->HandleEvent();
        HandleCallBack();
    }
}

void EventLoop::addCallBack(CallBack call) {
    if(call==nullptr){
        return;
    }
    Functor.push_back(call);
}

void EventLoop::HandleCallBack() {
    while (!Functor.empty()) {
        std::vector<CallBack> functors;
        { functors.swap(Functor); }

        for (const CallBack& functor : functors) {
            functor();  //这里的问题
        }
    }

	/*如果在functor()函数中进行了Functors中的插入，下面代码就有问题@wanghs*/
    // if(Functor.empty()){
    //     return;
    // }
    // auto begin=Functor.begin();
    // for(;begin!=Functor.end();){
    //     (*begin)();
    //     begin = Functor.erase(begin);
    // }

	/*修改方法2@wanghs*/
	// if(Functor.empty()){
    //     return;
    // }
    // auto begin=Functor.begin();
    // for(;begin!=Functor.end();){
    //     (*begin)();
    
	//		auto begin=Functor.begin();//增加这一句代码应该也可以@wanghs
    //     begin = Functor.erase(begin);
    // }
}

void EventLoop::callAfter(Time time, CallBack callback) {
    auto event = std::make_shared<Event>(m_CurrentTime + time);
    event->setITimeCallback(std::move(callback));
    m_EventHeap.push(std::move(event));
}

EventLoop& EventLoop::getInstance() {
    return instance_;
}