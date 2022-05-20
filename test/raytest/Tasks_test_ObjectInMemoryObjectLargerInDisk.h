#include<iostream>
#include "Node.h"
#include "RayBaseDefination.h"
#include "GlobalInformationClient.h"
#include "RayAPI.h"
#include "GlobalInformationServer.h"
#include "NodeManager.h"

enum ResourceType { CPU=1, MEM, PLASMA_MEM };  
Ray::ResourceSet Task_Test_RequestSum({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.8 } });
Ray::ResourceSet Task_Test_RequestAdd({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 1 } });
Ray::ResourceSet Task_Test_RequestResult({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.6 } });

Ray::GlobalInformationServer informationServer;
std::vector<std::shared_ptr<Ray::GlobalInformationClient>> informationClients;
std::vector<std::shared_ptr<Ray::RayService>> rayServices;

void TaskOne(std::shared_ptr<Ray::RayEnv> env,int a,int b,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=50;
    obj->data_=a+b;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskTwo(std::shared_ptr<Ray::RayEnv> env,int a,int b,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=50;
    obj->data_=a+b;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskThree(std::shared_ptr<Ray::RayEnv> env,int a,Ray::ObjectID id,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=50;
    obj->data_=a;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskFour(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=30;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskFive(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskSix(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,Ray::ObjectID idThree,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskSeven(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskEight(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,Ray::ObjectID idfour,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskNine(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idthree,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskTen(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskElevn(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,Ray::ObjectID idfour,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void Task12(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,Ray::ObjectID idfour,Ray::ObjectID five,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void Task14(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,CallBack call){
    env->reportFinish_(nullptr);
    call();
}

void Task13(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,Ray::ObjectID idfour,int instructNum){
    Ray::ObjectID Object14=env->service_->remote(Task_Test_RequestSum,TaskSix,idOne,idtwo,idthree,instructNum);
    env->service_->remote(Task_Test_RequestSum,Task14,Object14,[env](){
        env->reportFinish_(nullptr);
    });
}


void Task16(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,CallBack call,int instructNum){
    Ray::ObjectID Object16=env->service_->remote(Task_Test_RequestSum,TaskFive,idOne,idtwo,instructNum);
    env->service_->remote(Task_Test_RequestSum,Task14,Object16,[call,env](){
        env->reportFinish_(nullptr);
        call();
    });
}

void Task15(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    Ray::ObjectID Object15=env->service_->remote(Task_Test_RequestSum,TaskSix,idOne,idtwo,idthree,instructNum);
    env->service_->remote(Task_Test_RequestSum,Task16,Object15,idOne,[env](){
        env->reportFinish_(nullptr);
    },instructNum);
}

void Task18(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void Task19(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,Ray::ObjectID idfour,int instructNum,CallBack call){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    env->reportFinish_(obj);
    call();
}

void Task17(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    Ray::ObjectID Object17=env->service_->remote(Task_Test_RequestSum,Task18,idOne,idtwo,instructNum);
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    Ray::ObjectID object19=env->service_->remote(Task_Test_RequestSum,Task19,idOne,idtwo,idthree,Object17,instructNum,[obj,env](){
        env->reportFinish_(obj);
    });
}

void Task20(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    env->reportFinish_(nullptr);
}

void Task21(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    env->reportFinish_(obj);
}

void Task22(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    env->reportFinish_(nullptr);
}

void Task23(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=40;
    obj->data_=20;
    env->reportFinish_(obj);
}

void Task26(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,int instructNum,CallBack call){
    env->reportFinish_(nullptr);
    call();
}

void Task25(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,int instructNum,CallBack call){
    auto callback=[env,call](){
        std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
        obj->size_=40;
        obj->data_=20;
        env->reportFinish_(nullptr);
        call();
    };
    
    Ray::ObjectID Object25=env->service_->remote(Task_Test_RequestSum,Task26,idOne,instructNum,callback);
}

void Task24(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idtwo,Ray::ObjectID idthree,int instructNum){
    auto call=[env](){
        env->reportFinish_(nullptr);
    };
    
    Ray::ObjectID Object24=env->service_->remote(Task_Test_RequestSum,Task25,idOne,idtwo,instructNum,call);
}

void TestTaskObjectInDisk_Driver(NodeID nodeid,int a,int b,int instructNum){
    //3
    Ray::ObjectID ObjectOne_=rayServices[nodeid]->remote(Task_Test_RequestSum,TaskOne,a,b,instructNum);
    //1
    Ray::ObjectID ObjectTwo_=rayServices[nodeid]->remote(Task_Test_RequestAdd,TaskTwo,a,b,instructNum);
    //1
    Ray::ObjectID ObjectThree_=rayServices[nodeid]->remote(Task_Test_RequestAdd,TaskThree,a,ObjectTwo_,instructNum);
    //1
    Ray::ObjectID ObjectFour_=rayServices[3]->remote(Task_Test_RequestSum,TaskFour,ObjectOne_,ObjectThree_,instructNum);
    // // //3
    Ray::ObjectID ObjectFive_=rayServices[3]->remote(Task_Test_RequestSum,TaskFive,ObjectThree_,ObjectFour_,instructNum);

    Ray::ObjectID ObjectSix_=rayServices[2]->remote(Task_Test_RequestSum,TaskSix,ObjectThree_,ObjectFour_,ObjectOne_,instructNum);

    Ray::ObjectID ObjectSeven_=rayServices[4]->remote(Task_Test_RequestSum,TaskSeven,ObjectSix_,instructNum);

    // Ray::ObjectID ObjectEight_=rayServices[1]->remote(Task_Test_RequestSum,TaskEight,ObjectSeven_,ObjectThree_,ObjectSix_,ObjectFive_,instructNum);

    // Ray::ObjectID ObjectNine_=rayServices[3]->remote(Task_Test_RequestSum,TaskNine,ObjectSeven_,ObjectEight_,instructNum);

    // Ray::ObjectID ObjectTen_=rayServices[2]->remote(Task_Test_RequestSum,TaskTen,ObjectSeven_,ObjectEight_,ObjectNine_,instructNum);

    // Ray::ObjectID ObjectElevn_=rayServices[4]->remote(Task_Test_RequestSum,TaskElevn,ObjectSeven_,ObjectEight_,ObjectNine_,ObjectTen_,instructNum);

    // Ray::ObjectID Object12_=rayServices[1]->remote(Task_Test_RequestSum,Task12,ObjectSeven_,ObjectEight_,ObjectNine_,ObjectTen_,ObjectTwo_,instructNum);

    // //空
    // Ray::ObjectID Object13_=rayServices[3]->remote(Task_Test_RequestSum,Task13,ObjectEight_,ObjectNine_,ObjectTen_,ObjectTwo_,instructNum);

    // //空
    // Ray::ObjectID Object14_=rayServices[2]->remote(Task_Test_RequestSum,Task13,ObjectEight_,ObjectNine_,ObjectTen_,ObjectTwo_,instructNum);

    // //空
    // Ray::ObjectID Object15_=rayServices[4]->remote(Task_Test_RequestSum,Task15,Object12_,ObjectTen_,ObjectTwo_,instructNum);

    // Ray::ObjectID Object16_=rayServices[1]->remote(Task_Test_RequestSum,Task17,Object12_,ObjectTen_,ObjectTwo_,instructNum);

    // //空
    // Ray::ObjectID Object17_=rayServices[3]->remote(Task_Test_RequestSum,Task20,Object13_,instructNum);

    // Ray::ObjectID Object18_=rayServices[2]->remote(Task_Test_RequestSum,Task21,Object15_,Object17_,instructNum);

    // //空
    // Ray::ObjectID Object19_=rayServices[4]->remote(Task_Test_RequestSum,Task22,Object15_,Object17_,Object14_,instructNum);

    // Ray::ObjectID Object20_=rayServices[1]->remote(Task_Test_RequestSum,Task23,Object18_,Object17_,Object19_,instructNum);

    // //空
    // Ray::ObjectID Object21_=rayServices[3]->remote(Task_Test_RequestSum,Task24,Object19_,Object16_,Object12_,instructNum);
}   

