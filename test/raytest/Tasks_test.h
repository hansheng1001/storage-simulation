#include<iostream>
#include "Node.h"
#include "RayBaseDefination.h"
#include "GlobalInformationClient.h"
#include "RayAPI.h"
#include "GlobalInformationServer.h"
#include "NodeManager.h"

enum ResourceType { CPU=1, MEM, PLASMA_MEM };  
Ray::ResourceSet Task_Test_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 90 } });

Ray::GlobalInformationServer informationServer;
std::vector<std::shared_ptr<Ray::GlobalInformationClient>> informationClients;
std::vector<std::shared_ptr<Ray::RayService>> rayServices;

void Sum(std::shared_ptr<Ray::RayEnv> env,int a,int b,int instructNum){
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

void TestTask_Driver(NodeID nodeid,int a,int b,int instructNum){
    rayServices[nodeid]->remote(Task_Test_Request,Sum, a, b, instructNum);
    //这里就是将sum这个写入到本地磁盘上，这个不急测
}


