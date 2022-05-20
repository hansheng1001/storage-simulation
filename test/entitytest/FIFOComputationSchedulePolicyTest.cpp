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
    ComputationTask task3{20000000, 0.4, 400 * 1024 * 1024};
    ComputationTask task4{100000000, 0.1, 10 * 1024 * 1024};
    ComputationTask task5{60000000, 0.2, 100 * 1024 * 1024};
    ComputationTask task6{40000000, 0.2, 200 * 1024 * 1024};
    ComputationTask task7{20000000, 0.4, 400 * 1024 * 1024};
    ComputationTask task8{40000000, 0.2, 200 * 1024 * 1024};

    ComputationArch arch;
    arch.sockets_.push_back({0.5, 0.25, 4, 0});
    arch.sockets_.push_back({0.5, 0.25, 4, 1});
    arch.memoryNodes_.push_back({600 * 1024 * 1024, 30.0});
    arch.memoryNodes_.push_back({600 * 1024 * 1024, 30.0});
    arch.NUMALatencyFactor_ = 2.0;

    auto scheduler = CreateFIFOComputationTaskScheduler(arch);

    //submit task and callback
    scheduler->submitCompuationTask(task1, [](){
        isTaskFinishAt(34000000);
    });

    scheduler->submitCompuationTask(task2, [](){
        isTaskFinishAt(68000000);
    });

    scheduler->submitCompuationTask(task3, [](){
        isTaskFinishAt(61500000);
    });

    scheduler->submitCompuationTask(task4, [](){
        isTaskFinishAt(86250000);
    });

    scheduler->submitCompuationTask(task5, [](){
        isTaskFinishAt(51000000);
    });
    
    scheduler->submitCompuationTask(task6, [](){
        isTaskFinishAt(68000000);
    });

    scheduler->submitCompuationTask(task7, [](){
        isTaskFinishAt(123000000);
    });

    scheduler->submitCompuationTask(task8, [](){
        isTaskFinishAt(102000000);
    });

    EventLoop::getInstance().ChooseEventToStart();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();   
}