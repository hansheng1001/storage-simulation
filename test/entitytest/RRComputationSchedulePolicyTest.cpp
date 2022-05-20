#include "ComputationTaskSchedulerFactory.h"
#include "EventLoop.h"
#include <gtest/gtest.h>

void isTaskFinishAt(Time expectTime)
{
    EXPECT_EQ(EventLoop::getInstance().GetCurrentTime(), expectTime);
}

TEST(ComputationTaskSchedule, FIFOPolicy)
{
    ComputationTask task1{40000000, 0.2, 200 * 1024 * 1024};
    ComputationTask task2{80000000, 0.2, 400 * 1024 * 1024};
    ComputationTask task3{30000000, 0.4, 400 * 1024 * 1024};
    ComputationTask task4{50000000, 0.1, 300 * 1024 * 1024};
    ComputationTask task5{60000000, 0.2, 100 * 1024 * 1024};

    ComputationArch arch;
    arch.sockets_.push_back({0.5, 0.25, 2, 0});
    arch.sockets_.push_back({0.5, 0.25, 2, 1});
    arch.memoryNodes_.push_back({600 * 1024 * 1024, 30.0});
    arch.memoryNodes_.push_back({600 * 1024 * 1024, 30.0});
    arch.NUMALatencyFactor_ = 2.0;

    auto scheduler = CreateRRComputationTaskScheduler(arch, 20000000);

    //submit task and callback
    scheduler->submitCompuationTask(task1, [](){
        isTaskFinishAt(43375000);
    });

    scheduler->submitCompuationTask(task2, [](){
        isTaskFinishAt(85115385);
    });

    scheduler->submitCompuationTask(task3, [](){
        isTaskFinishAt(67250002);
    });

    scheduler->submitCompuationTask(task4, [](){
        isTaskFinishAt(47740384);
    });

    scheduler->submitCompuationTask(task5, [](){
        isTaskFinishAt(89750000);
    });
    
    EventLoop::getInstance().ChooseEventToStart();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();   
}