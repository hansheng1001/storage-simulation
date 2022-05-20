#include "DiskDriverFactory.h"
#include "EventLoop.h"
#include <gtest/gtest.h>

void isTaskFinishAt(Time expectTime)
{
    EXPECT_EQ(EventLoop::getInstance().GetCurrentTime(), expectTime);
}

TEST(DiskIOSchedule, FIFOPolicy)
{
    Disk disk{
        .channelNum_ = 4,
        .maxReadBandwidth_ = 500000000,
        .maxWriteBandWidth_ = 200000000,
        .maxReadBandWidthPerChannel_ = 200000000,
        .maxWriteBandWidthPerChannel_ = 100000000,
        .readBaseLatency_ = 1000000,
        .writeBaseLatency_ = 1000000
    };

    auto driver = CreateFIFODiskDriver(disk, true);

    DiskIOTask task1{
        .IOLength_ = 100000000,
        .direction_ = DiskIOTask::READ
    };

    DiskIOTask task2{
        .IOLength_ = 200000000,
        .direction_ = DiskIOTask::READ
    };

    DiskIOTask task3{
        .IOLength_ = 50000000,
        .direction_ = DiskIOTask::READ
    };

    DiskIOTask task4{
        .IOLength_ = 10000000,
        .direction_ = DiskIOTask::WRITE
    };

    DiskIOTask task5{
        .IOLength_ = 10000000,
        .direction_ = DiskIOTask::READ
    };

    driver->submitDiskIOTask(task1, [](){
        isTaskFinishAt(801000000);
    });

    driver->submitDiskIOTask(task2, [](){
        isTaskFinishAt(1601000000);
    });

    driver->submitDiskIOTask(task3, [](){
        isTaskFinishAt(401000000);
    });

    driver->submitDiskIOTask(task4, [](){
        isTaskFinishAt(201000000);
    });

    driver->submitDiskIOTask(task5, [](){
        isTaskFinishAt(282000000);
    });

    EventLoop::getInstance().ChooseEventToStart();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();   
}