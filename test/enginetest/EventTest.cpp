#include"Event.h"

#include<gtest/gtest.h>

const int c=4;
void isHandleOK(int a){
    EXPECT_EQ(a,c);
}

TEST(EventTest,CreateEvent){
    Event event(c);
    event.setITimeCallback(std::bind(isHandleOK,c));

    event.HandleEvent();
    EXPECT_EQ(event.getTime(),c);
}

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}