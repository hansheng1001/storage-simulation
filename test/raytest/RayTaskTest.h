#include<iostream>
#include "Node.h"
#include "RayBaseDefination.h"
#include "GlobalInformationClient.h"
#include "RayAPI.h"
#include "GlobalInformationServer.h"
#include "NodeManager.h"

#include<memory>

extern int taskNumber;
enum ResourceType { CPU=1, MEM, PLASMA_MEM }; 
Ray::ResourceSet Task_Test_One({ { CPU, 1.0 }, { MEM, 8 }, { PLASMA_MEM, 10 } });
Ray::ResourceSet Task_Test_Two({ { CPU, 1.0 }, { MEM, 8 }, { PLASMA_MEM, 5 } });
Ray::ResourceSet Task_Test_Three({ { CPU, 1.0 }, { MEM, 8 }, { PLASMA_MEM, 5 } });
Ray::ResourceSet Task_Test_Four({ { CPU, 1.0 }, { MEM, 8 }, { PLASMA_MEM, 5 } });

Ray::GlobalInformationServer informationServer(0);
std::vector<std::shared_ptr<Ray::GlobalInformationClient>> informationClients;
std::vector<std::shared_ptr<Ray::RayService>> rayServices;

void stopGlobalInfomation();

void TaskOneChildOne(std::shared_ptr<Ray::RayEnv> env,int a,int b,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=200*1024;
    obj->data_=a+b;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=1ul<<24;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskOneChildTwo(std::shared_ptr<Ray::RayEnv> env,int a){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=100*1024;
    obj->data_=a;

    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    node->submitTask(direction_,obj->size_,[env,obj](){
        env->reportFinish_(obj);
    });
}

void TaskOneChildThree(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,int b,CallBack call){
    env->reportFinish_(nullptr);
    call();
}

void TaskOne(std::shared_ptr<Ray::RayEnv> env,int a,int b){
    Ray::ObjectID ObjectOne_=env->service_->remote(Task_Test_One,TaskOneChildOne,a,b,100000000);
    Ray::ObjectID ObjectTwo_=env->service_->remote(Task_Test_Two,TaskOneChildTwo,a);
    Ray::ObjectID ObjectThree_=env->service_->remote(Task_Test_Three,TaskOneChildThree,ObjectOne_,ObjectTwo_,b,[env](){
        env->reportFinish_(nullptr);
        stopGlobalInfomation();
    });
}

void TaskTwoChildOne(std::shared_ptr<Ray::RayEnv> env,int instructNum,CallBack call){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=50*1024;
    
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=1ul<<20;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj,call](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
        call();
    });
}

//这里直接是一个CPU的任务
void TaskTwo(std::shared_ptr<Ray::RayEnv> env){
    Ray::ObjectID ObjectOne_=env->service_->remote(Task_Test_One,TaskTwoChildOne,100000000,[env](){
        env->reportFinish_(nullptr);
        stopGlobalInfomation();
    });
}

void TaskThreeChildOne(std::shared_ptr<Ray::RayEnv> env,int size,CallBack call){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=size;
    DiskIOTask::Direction direction_=DiskIOTask::Direction::READ;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    node->submitTask(direction_,obj->size_,[env,obj,call](){
        env->reportFinish_(obj);
        call();
    });
}

//这里直接提交一个I/O任务
void TaskThree(std::shared_ptr<Ray::RayEnv> env,int size){
    Ray::ObjectID ObjectOne_=env->service_->remote(Task_Test_One,TaskThreeChildOne,size,[env](){
        env->reportFinish_(nullptr);
        stopGlobalInfomation();
    });
}

//递交一个CPU类型的任务
void TaskFourChildOne(std::shared_ptr<Ray::RayEnv> env,int a,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=300*1024;
    obj->data_=a;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=(1ul<<28);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
        //将该对象放入到磁盘中 
    });
}

void TaskFourChildTwo(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID id){
    env->reportFinish_(nullptr);
}

void TaskFourChildThree(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,CallBack call){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    if(idOne.getObj()!=nullptr){
        obj->size_=idOne.getObj()->size_;
    }
    if(idTwo.getObj()!=nullptr){
        obj->size_+=idTwo.getObj()->size_;
    }
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    node->submitTask(direction_,obj->size_,[env,obj,call](){
        env->reportFinish_(obj);
        call();
    });
}

void TaskFour(std::shared_ptr<Ray::RayEnv> env,int a,int b){
    Ray::ObjectID ObjectOne_=env->service_->remote(Task_Test_One,TaskFourChildOne,a,100000000);
    Ray::ObjectID ObjectTwo_=env->service_->remote(Task_Test_Two,TaskFourChildTwo,ObjectOne_);
    Ray::ObjectID ObjectThree_=env->service_->remote(Task_Test_Three,TaskFourChildThree,ObjectOne_,ObjectTwo_,[env](){
        env->reportFinish_(nullptr);
        stopGlobalInfomation();
    });
}