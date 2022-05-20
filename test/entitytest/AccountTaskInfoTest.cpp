#include <gtest/gtest.h>
#include <fstream>
#include "AccountTaskInfo.h"

TEST(AccountTaskInfo, testAccountTaskInfo)
{
    std::ofstream file{ "TaskAccount.json",std::ios::trunc };
    AccountTaskInfo::getInstance().SetOutputStream(&file);
    //task(0,0)：step1
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskSubmit, 0, 0, 1);
    //task(0,1)：step1
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskSubmit, 0, 1, 1);
    //task(0,0)：step2
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskResourceSatisfy, 0, 0, 5);
    //task(0,1)：step2
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskResourceSatisfy, 0, 1, 5);
    //task(0,0)：step3
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskScheduled, 0, 0, 10, 3);
    //task(0,1)：step3
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskScheduled, 0, 1, 10, 5);
    //task(0,1)：step4
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskStartExec, 0, 1, 12);
    //task(0,1)：step5
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskEndExec, 0, 1, 20);
    //task(0,0)：step4
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskStartExec, 0, 0, 14);
    //task(0,0)：step5
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskEndExec, 0, 0, 20);

    
    file.close();
    AccountTaskInfo::putInstance();

}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}