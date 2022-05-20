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

void SumObject(std::shared_ptr<Ray::RayEnv> env,int a,int b,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=30;
    obj->data_=a+b;
    //这是一个CPU密集型任务，提交到CPU上
    ComputationTask task;
    task.instructionNum_=instructNum;
    task.memoryRequired_=10*sizeof(int);
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    node->submitCPUTask(task,[env,obj](){
        env->reportFinish_(obj);
    });
}

void addObject(std::shared_ptr<Ray::RayEnv> env,int a,Ray::ObjectID id,int instructNum){
    std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
    obj->size_=21;
    obj->data_=90;
    //这是一个I/O密集型任务，提交到磁盘上
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    // node->submitCPUTask(task,[env,obj](){
    //     env->reportFinish_(obj);
    // });
    node->submitTask(direction_,obj->size_,[env,obj](){
        env->reportFinish_(obj);
    });
}

void ResultSizeLarger(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID idOne,Ray::ObjectID idTwo,int instructNum){
    env->reportFinish_(nullptr);
}

void TestTaskSuccess_Driver(NodeID nodeid,int a,int b,int instructNum){
    Ray::ObjectID sumObject_=rayServices[nodeid]->remote(Task_Test_RequestSum,SumObject, a, b, instructNum);
    //这里就是将sum这个写入到本地磁盘上，这个不急测
    Ray::ObjectID addobject_=rayServices[nodeid]->remote(Task_Test_RequestAdd,addObject,a,sumObject_,instructNum);

    //rayServices[nodeid]->remote(Task_Test_RequestResult,ResultSizeLarger,sumObject_,addobject_,2000);
}