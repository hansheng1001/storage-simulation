#include"EventLoop.h"
#include"Event.h"

#include<gtest/gtest.h>

const int test=9;
int i=1;

void isTaskFinishAt(Time expectTime)
{
    EXPECT_EQ(EventLoop::getInstance().GetCurrentTime(), expectTime);
}

void handleEvent(int time){
    isTaskFinishAt(time);
}

TEST(ScheduleTest,EventTest){
    EXPECT_EQ(EventLoop::getInstance().GetCurrentTime(),0);

    for(;i<100;i+=2){
        EventLoop::getInstance().callAfter(i,std::bind(handleEvent,i));
    }
	
    EventLoop::getInstance().ChooseEventToStart();
}

TEST(ScheduleTest,CallBackTest){
    int t=EventLoop::getInstance().GetCurrentTime();
    int b;
    auto f=[](int & c){ c=test;};

    EventLoop::getInstance().callAfter(i,std::bind(handleEvent,i+t));
    EventLoop::getInstance().addCallBack(std::bind(f,std::ref(b)));
    EventLoop::getInstance().ChooseEventToStart();
    EXPECT_EQ(b,test);
}

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}