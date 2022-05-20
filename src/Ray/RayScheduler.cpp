#include "RayScheduler.h"
#include "EventLoop.h"
#include "Node.h"
#include "NodeManager.h"
#include "SchedulingPolicy.h"

enum TaskMessageCategory { SENDTASKTOSCHEDULE, TaskScheduleSuccess, TaskScheduleFail, SendDataReady, ExecuateFinish };

struct TaskMsg {
    /* data */
    TaskMessageCategory category;
    Time                time;
    NodeID              nodeID;
    void *              content;
};

Ray::RayScheduler::RayScheduler(NodeID localNodeID) : localNodeID_(localNodeID) {}

const Ray::LocalResourceTable& Ray::RayScheduler::getLocalResourceInfo() const {
    return localResourceTable_;
}
std::shared_ptr<Ray::GlobalResourceTable> Ray::RayScheduler::getGlobalResourceInfo(){
    return pGlobalResourceTable_;
}

Time Ray::RayScheduler::getTime() {
    return EventLoop::getInstance().GetCurrentTime();
}

void Ray::RayScheduler::startClient(NodeID nodeID, CallBack call) {
    std::shared_ptr<Node> nodeServer = NodeManager::getIntance().searchNode(nodeID);
    std::shared_ptr<Node> nodeClient = NodeManager::getIntance().searchNode(localNodeID_);
    if (nodeServer==nullptr&& nodeClient==nullptr) {
        return;
    }
    Connection connect;
    if (connectionsClient_[nodeID].isConnect) {
        connect = connectionsClient_[nodeID].connection;
    }
    else {
        nodeServer->listenRayService();
        nodeClient->listenRayService();
        Port port            = nodeClient->CreatePort();
        connect              = nodeClient->CreateConnection(localNodeID_, port, nodeID, Serverport::RayServicePort);
        
        ConnectionsClient_ client;
        client.connection=connect;
        client.id=nodeID;
        client.isConnect=false;
        connectionsClient_.insert(std::pair<NodeID,ConnectionsClient_>(nodeID,client));
        
        
        auto connectCallback = [nodeClient, this, call](Connection connection) {
            this->push(connection);
            if (call) {
                call();
            }
        };
        nodeClient->connect(connect, std::bind(connectCallback, std::placeholders::_1));
    }
}

void Ray::RayScheduler::startServer() {
    std::shared_ptr<Node> node       = NodeManager::getIntance().searchNode(localNodeID_);
    auto acceptCall = [this,node](Connection connection){ 
        this->push(connection);
        //这里要设置read所对应的回调函数
        auto call=[this](Message msg){
            this->TaskHandleMessage(msg);
        };
        node->read(connection,call);
    };
    node->listenRayService();
    node->accept(std::bind(acceptCall, std::placeholders::_1),Serverport::RayServicePort);
}

void Ray::RayScheduler::submitTask(RayTaskHandle task) {
    //首先对任务的特性选取节点调度
    //选取节点开始调度的时候会返回一个节点ID
    NodeID id = schedulingPolicy_->getBestNodeToSchedule(task);
    if (id != localNodeID_) {
        sendRequestToRemoteScheduler(id, task);
    }
    else {
        acceptNewTaskFromRemoteScheduler(task);  //这个就是直接执行
    }
}

void Ray::RayScheduler::TaskHandleMessage(Message message) {
    TaskMsg msg = *(TaskMsg*)message.buff;
    delete (TaskMsg*)message.buff;
    switch (msg.category) {
        case SENDTASKTOSCHEDULE:{
            RayTaskHandle task=*(RayTaskHandle*)msg.content;
            delete (RayTaskHandle*)msg.content;
            handleRemoteRequest(msg.nodeID, task);
            break;
        }
        case TaskScheduleSuccess: {
            RayTaskHandle task=*(RayTaskHandle*)msg.content;
            delete (RayTaskHandle*)msg.content;
            schedulingPolicy_->updateTaskStatus(task->taskID_, msg.nodeID, TaskStatus::ACCEPT, msg.time);
            //handleRemoteResponse(true, task);
            break;
        }
        case TaskScheduleFail: {
            RayTaskHandle task=*(RayTaskHandle*)msg.content;
            delete (RayTaskHandle*)msg.content;
            schedulingPolicy_->updateTaskStatus(task->taskID_, msg.nodeID, TaskStatus::DENY, msg.time);
            handleRemoteResponse(false, task);
            break;
        }
        case SendDataReady: {
            RayTaskHandle task=*(RayTaskHandle*)msg.content;
            delete (RayTaskHandle*)msg.content;
            schedulingPolicy_->updateTaskStatus(task->taskID_, msg.nodeID, TaskStatus::PULL_FINISH, msg.time);
            break;
        }
        case ExecuateFinish: {
            RayTaskHandle task=*(RayTaskHandle*)msg.content;
            delete (RayTaskHandle*)msg.content;
            schedulingPolicy_->updateTaskStatus(task->taskID_, msg.nodeID, TaskStatus::DONE, msg.time);
            break;
        }
        default:
            break;
    }
}

void Ray::RayScheduler::sendRequestToRemoteScheduler(NodeID nodeID, RayTaskHandle task) {
    
    //首先进行查询
    if (connectionsClient_.find(nodeID) == connectionsClient_.end()) {
        startClient(nodeID, [nodeID, task, this]() { this->sendRequestToRemoteScheduler(nodeID, task); });
        return;
    }
    else{
        if(connectionsClient_.find(nodeID)->second.isConnect==false){
            connectionsClient_.find(nodeID)->second.calls.push_back([nodeID, task, this](){
                this->sendRequestToRemoteScheduler(nodeID, task);
            });
            return;
        }
    }
    std::shared_ptr<Node> nodeLocal = NodeManager::getIntance().searchNode(localNodeID_);
    if (nodeLocal==nullptr) {
        return;
    }
    RayTaskHandle* t=new RayTaskHandle(task);
    TaskMsg* msgScheduler  = new TaskMsg();
    msgScheduler->category = SENDTASKTOSCHEDULE;
    msgScheduler->content  = t;
    msgScheduler->nodeID   = localNodeID_;
    msgScheduler->time     = getTime();

    Message message;
    message.buff       = msgScheduler;
    message.bufflength = sizeof(RayTaskHandle);
    message.size       = 256;

    nodeLocal->write(connectionsClient_[nodeID].connection, message, nullptr);
    nodeLocal->read(connectionsClient_[nodeID].connection, [this,nodeLocal,nodeID](Message message) { 
        this->TaskHandleMessage(message); 
        nodeLocal->read(connectionsClient_[nodeID].connection,[this,nodeLocal](Message message){
            this->TaskHandleMessage(message);
        });
    });
}

bool Ray::RayScheduler::isLocalResourceSatisfied(const ResourceSet& resourceRequest) {
    for (auto resource : resourceRequest.resources_) {
        float available = table_[resource.first];
        if (resource.second > available) {
            return false;
        }
    }
    return true;
}

void Ray::RayScheduler::sendTaskScheduleResult(NodeID nodeID, RayTaskHandle task, bool permitted) {
    if (connectionsClient_.find(nodeID) == connectionsClient_.end()) {
        startClient(nodeID,
                    [this, nodeID, task, permitted]() { this->sendTaskScheduleResult(nodeID, task, permitted); });
        return;
    }
    else{
        if(connectionsClient_.find(nodeID)->second.isConnect==false){
            connectionsClient_.find(nodeID)->second.calls.push_back([this,nodeID,task,permitted](){
                this->sendTaskScheduleResult(nodeID, task, permitted);
            });
            return;
        }
    }

    RayTaskHandle* t=new RayTaskHandle(task);
    TaskMsg* msgResult = new TaskMsg();
    Message  message;
    if (permitted) {
        msgResult->category      = TaskScheduleSuccess;
        msgResult->time          = getTime();
        msgResult->nodeID        = localNodeID_;
        msgResult->content       = t;
        message.bufflength = 1;
    }
    else {
        msgResult->category = TaskScheduleFail;
        msgResult->time     = getTime();
        msgResult->nodeID   = localNodeID_;
        msgResult->content  = t;

        message.bufflength = sizeof(RayTaskHandle);
    }

    message.buff = msgResult;
    message.size = 256;

    std::shared_ptr<Node> nodeLocal = NodeManager::getIntance().searchNode(localNodeID_);

    nodeLocal->write(connectionsClient_[nodeID].connection, message, nullptr);
    nodeLocal->read(connectionsClient_[nodeID].connection, [this,nodeLocal,nodeID](Message message) { 
        this->TaskHandleMessage(message); 
        nodeLocal->read(connectionsClient_[nodeID].connection,[this,nodeLocal](Message message){
            this->TaskHandleMessage(message);
        });
    });
}

void Ray::RayScheduler::handleRemoteRequest(NodeID peerNodeID, RayTaskHandle task) {
    if (isLocalResourceSatisfied(task->resourceRequest_)) {
        sendTaskScheduleResult(peerNodeID, task, true);
        acceptNewTaskFromRemoteScheduler(task);
    }
    else {
        sendTaskScheduleResult(peerNodeID, task, false);
    }
}

void Ray::RayScheduler::handleRemoteResponse(bool permitted, RayTaskHandle task) {
    if (permitted) {
        return;
    }
    else {
        //this->submitTask(task);
        EventLoop::getInstance().addCallBack([this, task]() { 
            this->submitTask(task);
        });
        //EventLoop::getInstance().addCallBack(std::bind(Ray::RayScheduler::submitTask,this,task));
    }
}

void Ray::RayScheduler::execWhenAcceptNewTaskFromRemote(NewTaskCallback callback) {
    whenAcceptNewTaskFromRemoteScheduler_ = callback;
}

bool Ray::RayScheduler::allocResource(const ResourceSet& resourceRequest) {
    bool schedulable = isLocalResourceSatisfied(resourceRequest);  // 不能边判断是否可调度，边分配资源
    if (!schedulable)
        return false;
    for (auto resource : resourceRequest.resources_) {
        float available = localResourceTable_[resource.first];
        if (resource.second > available) {
            localResourceTable_[resource.first] = 0;
            //std::cerr << "不应该出现的问题: 判断为可调度但资源却不够" << std::endl;
        }
        else {
            localResourceTable_[resource.first] -= resource.second;
        }
    }
    return true;
}

void Ray::RayScheduler::freeResource(const ResourceSet& resourceToFree) {
    for (auto resource : resourceToFree.resources_) {
        localResourceTable_[resource.first] += resource.second;
    }
}

void Ray::RayScheduler::sendTaskDataReadyToRemoteScheduler(NodeID nodeID, RayTaskHandle task) {
    if (connectionsClient_.find(nodeID) == connectionsClient_.end()) {
        startClient(nodeID, [this, nodeID, task]() { this->sendTaskDataReadyToRemoteScheduler(nodeID, task); });
        return;
    }
    else{
        if(connectionsClient_.find(nodeID)->second.isConnect==false){
            connectionsClient_.find(nodeID)->second.calls.push_back([this, nodeID, task](){
                this->sendTaskDataReadyToRemoteScheduler(nodeID, task);
            });
            return;
        }
    }
    TaskMsg* msgDataReady = new TaskMsg();
    RayTaskHandle* t=new RayTaskHandle(task);
    Message  message;

    msgDataReady->category = SendDataReady;
    msgDataReady->content  = t;
    msgDataReady->nodeID   = localNodeID_;
    msgDataReady->time     = getTime();

    message.buff       = msgDataReady;
    message.bufflength = sizeof(RayTaskHandle);
    message.size       = 256;


    std::shared_ptr<Node> nodeLocal = NodeManager::getIntance().searchNode(localNodeID_);

    nodeLocal->write(connectionsClient_[nodeID].connection, message, nullptr);
    nodeLocal->read(connectionsClient_[nodeID].connection, [this,nodeLocal,nodeID](Message message) { 
        this->TaskHandleMessage(message); 
        nodeLocal->read(connectionsClient_[nodeID].connection,[this,nodeLocal](Message message){
            this->TaskHandleMessage(message);
        });
    });
}

void Ray::RayScheduler::sendTaskExecuateFinish(NodeID nodeID, RayTaskHandle task) {
    if (connectionsClient_.find(nodeID) == connectionsClient_.end()) {
        startClient(nodeID, [this, nodeID, task]() { this->sendTaskExecuateFinish(nodeID, task); });
        return;
    }
    else{
        if(connectionsClient_.find(nodeID)->second.isConnect==false){
            connectionsClient_.find(nodeID)->second.calls.push_back([this, nodeID, task](){
                this->sendTaskExecuateFinish(nodeID, task);
            });
            return;
        }
    }
    TaskMsg* msg = new TaskMsg();
    RayTaskHandle* t=new RayTaskHandle(task);
    Message  message;

    msg->category = ExecuateFinish;
    msg->content  = t;
    msg->nodeID   = localNodeID_;
    msg->time     = getTime();

    message.buff       = msg;
    message.bufflength = sizeof(RayTaskHandle);
    message.size       = 256;

    // 帮你添了一句
    std::shared_ptr<Node> nodeLocal = NodeManager::getIntance().searchNode(localNodeID_);

    nodeLocal->write(connectionsClient_[nodeID].connection, message, nullptr);
    nodeLocal->read(connectionsClient_[nodeID].connection, [this,nodeLocal,nodeID](Message message) { 
        this->TaskHandleMessage(message); 
        nodeLocal->read(connectionsClient_[nodeID].connection,[this,nodeLocal](Message message){
            this->TaskHandleMessage(message);
        });
    });
}

void Ray::RayScheduler::setSchedulePolicy(){
    schedulingPolicy_=std::make_shared<Ray::SchedulingPolicy>(localNodeID_);
}

void Ray::RayScheduler::exit(){
    connectionsClient_.clear();
}

