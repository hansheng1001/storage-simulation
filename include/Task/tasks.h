// template <typename TaskFuncType, typename... Params>
//         auto remote(const std::vector<ResourceRequirement> &resourceRequirements, TaskFuncType taskfunc, Params
//&&...params)
//             -> typename std::enable_if<std::is_void<decltype(taskfunc(params...))>::value, ObjectID>::type

#include "RayBaseDefination.h"
#include "ObjectTableManager.h"
#include "RayAPI.h"
#include "NodeManager.h"
#include "BaseTask.h"
#include "Node.h"
#include "Computation.h"

#include "PUserObjectTable.h"
#include "EventLoop.h"
//我们假设0号节点对应的P层，也就是存放数据的
//相对应的，0号ObjectTable存放着0号
#include<vector>
#include <boost/any.hpp>

const size_t slice = 300;

std::vector<std::shared_ptr<Ray::ObjectTableManager>> tableManagers;
std::vector<std::shared_ptr<Ray::RayService>> rayServices;
std::vector<std::shared_ptr<Ray::SharedObjectStorage>> sharedStorages;
PUserObjectTable ptable;

enum ResourceType {CPU, MEM, PLASMA_MEM};

std::vector<Ray::ResourceRequirement> GC_ReadIndex_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

const std::vector<Ray::ResourceRequirement> GC_MarkDeletion_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

const std::vector<Ray::ResourceRequirement> GC_WriteData_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

void GC_Driver(size_t objSize,NodeID nodeid,unsigned long instructionNum) {//这里的ray所对应的应该在
    // IndexInfo 从哪里来（数据布局问题）
    std::shared_ptr<Ray::RayEnv> GCReadIndexEnv = std::make_shared<Ray::RayEnv>();
    GCReadIndexEnv->service_=nullptr;
    GCReadIndexEnv->reportFinish_=nullptr;
    Ray::ObjectID readIndexResultID=rayServices[nodeid]->remote(GC_ReadIndex_Requirement,
    std::bind(GC_ReadIndex_Func,GCReadIndexEnv,objSize,nodeid),GCReadIndexEnv,objSize,nodeid);//这里的global应该记录在日志当中，表明GC应该会收哪一个数据，这里的global是

    std::shared_ptr<Ray::RayEnv> GCMarkDeletionEnv = std::make_shared<Ray::RayEnv>();
    GCMarkDeletionEnv->service_=nullptr;
    GCMarkDeletionEnv->reportFinish_=nullptr;
    Ray::ObjectID
    markMarkDeletionResultID=rayServices[nodeid]->remote(GC_MarkDeletion_Requirement,std::bind(GC_MarkDeletions_Func,GCMarkDeletionEnv,readIndexResultID,readIndexResultID.getObj()->size_,instructionNum),readIndexResultID,nodeid);
}

// 从 P 层拉取数据和索引
void GC_ReadIndex_Func (std::shared_ptr<Ray::RayEnv> env,size_t objSize,NodeID nodeid) {
    std::shared_ptr<Ray::ObjectTableManager> objTable=tableManagers[nodeid];
    ptable.getObject(objSize,[nodeid,objTable,env](Ray::ObjectHandle handle){
        objTable->put(handle);
        env->reportFinish_(handle);
    });

}

void GC_MarkDeletions_Func (std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID id,size_t size,unsigned long
instructionNum) {
    NodeID nodeid=id.getKey().getOwnerNodeID();
    while(size>0){
        std::shared_ptr<Ray::RayEnv> GCMarkDeletionEnv = std::make_shared<Ray::RayEnv>();
        GCMarkDeletionEnv->service_=nullptr;
        GCMarkDeletionEnv->reportFinish_=nullptr;
        Ray::ObjectID markMarkDeletionResultID=rayServices[nodeid]->remote(GC_MarkDeletion_Requirement,
        std::bind(GC_MarkDeletion_Func,GCMarkDeletionEnv,id,slice,instructionNum),id,slice,instructionNum);

        std::shared_ptr<Ray::RayEnv> GCWriteDataEnv = std::make_shared<Ray::RayEnv>();
        GCWriteDataEnv->service_=nullptr;
        GCWriteDataEnv->reportFinish_=nullptr;
        Ray::ObjectID
        writeDataResultID=rayServices[nodeid]->remote(GC_WriteData_Requirement,std::bind(GC_WriteData_Func,GCWriteDataEnv,markMarkDeletionResultID,slice),GCWriteDataEnv,markMarkDeletionResultID,slice);
        size-=slice;
    }
}

void GC_MarkDeletion_Func (std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID id,size_t size,unsigned long
instructionNum){
    //这里要定义生成新的Object
    Ray::Object object;
    NodeID nodeid=id.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    std::shared_ptr<Ray::Object> getObjectMarkDeletion=std::make_shared<Ray::Object>(object);
    ComputationTask task;
    task.instructionNum_=instructionNum;
    task.memoryRequired_=size;
    node->submitCPUTask(task,[env,getObjectMarkDeletion](){
        env->reportFinish_(getObjectMarkDeletion);
    });
}

void GC_WriteData_Func (std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID id,size_t size){
    NodeID nodeid=id.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    node->submitTask(direction_,size,[env](){
        env->reportFinish_(nullptr);
    });
}

std::vector<Ray::ResourceRequirement> RemoveDup_ReadIndex_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

const std::vector<Ray::ResourceRequirement> RemoveDup_Execuate_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

void RemoveDup_Driver(size_t objSize,NodeID nodeid,unsigned long instructionNum){
    std::shared_ptr<Ray::RayEnv> RemoveDupReadIndexEnv = std::make_shared<Ray::RayEnv>();
    RemoveDupReadIndexEnv->service_=nullptr;
    RemoveDupReadIndexEnv->reportFinish_=nullptr;
    Ray::ObjectID readIndexResultID=rayServices[nodeid]->remote(RemoveDup_ReadIndex_Requirement,
    std::bind(RemoveDup_ReadIndex_Func,RemoveDupReadIndexEnv,nodeid,objSize),RemoveDupReadIndexEnv,nodeid,objSize);//这里的global应该记录在日志当中，表明GC应该会收哪一个数据，这里的global是

    std::shared_ptr<Ray::RayEnv> RemoveDupExecuateEnv = std::make_shared<Ray::RayEnv>();
    RemoveDupExecuateEnv->service_=nullptr;
    RemoveDupExecuateEnv->reportFinish_=nullptr;
    Ray::ObjectID
    readIndexResultID=rayServices[nodeid]->remote(RemoveDup_Execuate_Requirement,std::bind(RemoveDup_Execuate_Func,RemoveDupExecuateEnv,readIndexResultID,instructionNum),readIndexResultID,instructionNum);
}

void RemoveDup_ReadIndex_Func (std::shared_ptr<Ray::RayEnv> env,NodeID nodeid,size_t objSize){
    std::shared_ptr<Ray::ObjectTableManager> objTable=tableManagers[nodeid];
    ptable.getObject(objSize,[nodeid,objTable,env](Ray::ObjectHandle handle){
        objTable->put(handle);
        env->reportFinish_(handle);
    });
}

void RemoveDup_Execuate_Func(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID objID,unsigned long instructionNum){
    NodeID nodeid=objID.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);

    ComputationTask task;
    task.instructionNum_=instructionNum;
    task.memoryRequired_=objID.getObj()->size_;
    node->submitCPUTask(task,[env](){
        env->reportFinish_(nullptr);
    });
}

std::vector<Ray::ResourceRequirement> DupFirst_Execuate_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

void DupFirst_Driver(NodeID nodeid,size_t size){
    std::shared_ptr<Ray::RayEnv> DupFirstWriteEnv = std::make_shared<Ray::RayEnv>();
    DupFirstWriteEnv->service_=nullptr;
    DupFirstWriteEnv->reportFinish_=nullptr;
    Ray::ObjectID
    writeResultID=rayServices[nodeid]->remote(DupFirst_Execuate_Requirement,std::bind(DupFirst_WriteData_Func,DupFirstWriteEnv,size,nodeid),size,nodeid);
}

void DupFirst_WriteData_Func(std::shared_ptr<Ray::RayEnv> env,size_t size,NodeID
nodeid){//这里要传入nodeid和数据大小size
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    node->submitTask(direction_,size,[env](){
        env->reportFinish_(nullptr);
    });
}

std::vector<Ray::ResourceRequirement> DupSecond_ReadIndex_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

const std::vector<Ray::ResourceRequirement> DupSecond_Write_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

void DupSecond_Driver(NodeID nodeid,size_t size){
    std::shared_ptr<Ray::RayEnv> DupSecondReadIndexEnv = std::make_shared<Ray::RayEnv>();
    DupSecondReadIndexEnv->service_=nullptr;
    DupSecondReadIndexEnv->reportFinish_=nullptr;
    Ray::ObjectID
    readIndexResultID=rayServices[nodeid]->remote(DupSecond_ReadIndex_Requirement,std::bind(DupSecond_ReadIndex_Func,DupSecondReadIndexEnv,nodeid,size),size,nodeid);

    std::shared_ptr<Ray::RayEnv> DupSecondWriteEnv = std::make_shared<Ray::RayEnv>();
    DupSecondWriteEnv->service_=nullptr;
    DupSecondWriteEnv->reportFinish_=nullptr;
    Ray::ObjectID
    writeResultID=rayServices[nodeid]->remote(DupSecond_Write_Requirement,std::bind(DupSecond_Write_Func,DupSecondWriteEnv,readIndexResultID));
}

void DupSecond_ReadIndex_Func (std::shared_ptr<Ray::RayEnv> env,NodeID nodeid,size_t objSize) {
    std::shared_ptr<Ray::ObjectTableManager> objTable=tableManagers[nodeid];
    ptable.getObject(objSize,[nodeid,objTable,env](Ray::ObjectHandle handle){
        objTable->put(handle);
        env->reportFinish_(handle);
    });
}

void DupSecond_Write_Func(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID objID){
    NodeID nodeid=objID.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    size_t size=objID.getObj()->size_;
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    node->submitTask(direction_,size,[env](){
        env->reportFinish_(nullptr);
    });
}

std::vector<Ray::ResourceRequirement> EC_ReadIndex_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

std::vector<Ray::ResourceRequirement> EC_Execuate_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

const std::vector<Ray::ResourceRequirement> EC_Write_Requirement = {
    {CPU, 1.0},
    {MEM, 1.0},
    {PLASMA_MEM, 1.0}};

void EC_Driver(NodeID nodeid,size_t size,unsigned long instructionNum,bool isValid,size_trecoverySize){//传递是否有问题的标志
    std::shared_ptr<Ray::RayEnv> ECCReadIndexEnv = std::make_shared<Ray::RayEnv>();
    ECCReadIndexEnv->service_=nullptr;
    ECCReadIndexEnv->reportFinish_=nullptr;
    Ray::ObjectID
    readIndexResultID=rayServices[nodeid]->remote(EC_ReadIndex_Requirement,std::bind(ECC_ReadIndex_Func,ECCReadIndexEnv,nodeid,size),nodeid,size);

    std::shared_ptr<Ray::RayEnv> ECCExecuateEnv = std::make_shared<Ray::RayEnv>();
    ECCExecuateEnv->service_=nullptr;
    ECCExecuateEnv->reportFinish_=nullptr;
    Ray::ObjectID
    ECCExecuateResultID=rayServices[nodeid]->remote(EC_Execuate_Requirement,std::bind(ECC_Execuate_Func,ECCExecuateEnv,readIndexResultID,instructionNum,isValid,recoverySize),readIndexResultID,instructionNum,isValid,recoverySize);

    std::shared_ptr<Ray::RayEnv> ECCWriteEnv = std::make_shared<Ray::RayEnv>();
    ECCWriteEnv->reportFinish_=nullptr;
    ECCWriteEnv->service_=nullptr;
    Ray::ObjectID
    ECCWriteResultID=rayServices[nodeid]->remote(EC_Write_Requirement,std::bind(ECC_Write_Func,ECCWriteEnv,ECCExecuateResultID),ECCExecuateResultID);
}

void ECC_ReadIndex_Func(std::shared_ptr<Ray::RayEnv> env,NodeID nodeid,size_t objSize){
    std::shared_ptr<Ray::ObjectTableManager> objTable=tableManagers[nodeid];
    ptable.getObject(objSize,[nodeid,objTable,env](Ray::ObjectHandle handle){
        objTable->put(handle);
        env->reportFinish_(handle);
    });
}

void ECC_Execuate_Func(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID objID,unsigned long instructionNum,bool
isValid,size_t recoverySize){
    NodeID nodeid=objID.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    size_t size=objID.getObj()->size_;

    ComputationTask task;
    task.instructionNum_=instructionNum;
    task.memoryRequired_=objID.getObj()->size_;
    node->submitCPUTask(task,[env,isValid,objID,recoverySize](){
        if(isValid){
            env->reportFinish_(nullptr);
        }
        else{
            Ray::Object object;
            object.data_=recoverySize;
            object.data_=!isValid;
            std::shared_ptr<Ray::Object> Object=std::make_shared<Ray::Object>(object);
            env->reportFinish_(Object);
        }
    });
}

void ECC_Write_Func(std::shared_ptr<Ray::RayEnv> env,Ray::ObjectID objID){
    bool isValid=boost::any_cast<bool>(objID.getObj()->data_);
    if(isValid){
        return;
    }

    NodeID nodeid=objID.getKey().getOwnerNodeID();
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
    size_t size=objID.getObj()->size_;
    DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
    node->submitTask(direction_,size,[env](){
        env->reportFinish_(nullptr);
    });
}
