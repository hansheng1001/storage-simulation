#include"ObjectTableManager.h"
#include"Node.h"
#include"NodeManager.h"
#include"Connection.h"
#include"RayAPI.h"
#include"RayBaseDefination.h"
#include"DiskDriver.h"
#include"EventLoop.h"

#include"SharedObjectStorage.h"

#include<memory>

const size_t resultObjectMaxSize=40;
//做测试的时候content是有很大问题的，有极大可能会内存泄漏

enum MessageCategory{SendObjectInfo,receiveCompleteObjectInfo,SendObject,receiveCompleteObject,SendResult,ModifyCopies,DeleteObject};
struct Msg
{
    /* data */
    MessageCategory category;
    void*           content;
    void*           peer;
    void*           contentwo;
};

//对方收到了连接之后需要将连接加入到自己的connection
//已测
void Ray::ObjectTableManager::startClient(NodeID nodeid,CallBack call){
    std::shared_ptr<Node> nodeServer=NodeManager::getIntance().searchNode(nodeid);
    std::shared_ptr<Node> nodeClient=NodeManager::getIntance().searchNode(localNodeID_);
    if(nodeServer==nullptr&&nodeClient==nullptr){
        return;
    }

    Connection connect;
    if(connectionsClient_[nodeid].isConnect){
        connect=connectionsClient_[nodeid].connection;
    }
    else{
        nodeServer->listenObjectTable();
        nodeClient->listenObjectTable();
        Port port=nodeClient->CreatePort();
        connect=nodeClient->CreateConnection(localNodeID_,port,nodeid,Serverport::ObjectTablePort);
        ConnectionsClient_ client;
        client.connection=connect;
        client.id=nodeid;
        client.isConnect=false;
        connectionsClient_.insert(std::pair<NodeID,ConnectionsClient_>(nodeid,client));

        //这里是多变的
        auto connectCallback=[this,call](Connection connection){
            this->push(connection);
            if(call){
                call();
            }
        };
        nodeClient->connect(connect,std::bind(connectCallback,std::placeholders::_1));
    }
}

void Ray::ObjectTableManager::AcceptCallBack(Connection connection){
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    push(connection);
    node->read(connection,[this](Message message){
        this->HandleMessage(message);
    });
    node->accept([this](Connection conn){
        this->AcceptCallBack(conn);
    },Serverport::ObjectTablePort);
}

//已测
void Ray::ObjectTableManager::startServer(){
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    node->listenObjectTable();
    node->accept([this](Connection connection){
        this->AcceptCallBack(connection);
    },Serverport::ObjectTablePort);
}

void Ray::ObjectTableManager::HandleMessage(Message message){
    Msg msg=*(Msg*)message.buff;
    delete (Msg*)message.buff;

    switch(msg.category){
        case SendObjectInfo:{
            //okok
            ObjectID objectid=*(ObjectID*)msg.content;
            delete (ObjectID*)msg.content;
            RayService* peerRayService=(RayService*)msg.peer;
            receiveReadObjectInfoRequest(peerRayService,objectid);
            break;
        }
        case receiveCompleteObjectInfo:{
            //已测
            ObjectID objectid=*(ObjectID*)msg.content;
            delete (ObjectID*)msg.content;
            ObjectInfo info=*(ObjectInfo*)msg.peer;
            delete (ObjectInfo*)msg.peer;

            //找到在本地的info,进行修改
            ObjectInfo& localInfo=getInfomation(objectid.getKey());
            localInfo.isPrimary_=false;
            localInfo.obj_=nullptr;
            localInfo.objSize_=info.objSize_;
            //localInfo.status_=
            localInfo.usingCount_=info.usingCount_;

            localInfo.copies_.insert(localInfo.copies_.end(),info.copies_.begin(),info.copies_.end());
            
            localInfo.status_=Ray::ObjectTableManager::ObjectInfo::READABLE;
            receiveObjectInfo(objectid,message.size);
            break;
        }
        case SendObject:{
            //已测
            ObjectID objectid=*(ObjectID*)msg.content;
            delete (ObjectID*)msg.content;
            RayService* peerRayService=(RayService*)msg.peer;

            receiveReadObjectRequest(peerRayService,objectid);
            break;
        }
        case receiveCompleteObject:{
            ObjectID objID=*(ObjectID*)msg.peer;
            delete (ObjectID*)msg.peer;
            ObjectHandle obj=*(ObjectHandle*)msg.content;
            delete (ObjectHandle*)msg.content;
            receiveObject(objID,obj);
            break;
        }
        case ModifyCopies:{
            //已测
            RayService* peerRayService=(RayService*)msg.peer;
            ObjectKey key=*(ObjectKey*)msg.content;
            delete (ObjectKey*)msg.content;
            receiveCopyRemovedNotification(peerRayService,key);
            break;
        }
        case DeleteObject:{
            //已测
            ObjectKey key=*(ObjectKey*)msg.content;
            delete (ObjectKey*)msg.content;
            receiveRemovedObjectnotification(key);
            break;
        }
        case SendResult:
            //已测
            ObjectID objectid=*(ObjectID*)msg.content;
            delete (ObjectID*)msg.content;
            ObjectHandle handle=nullptr;
            if(msg.peer!=nullptr){
                handle=*(ObjectHandle*)msg.peer;
                delete (ObjectHandle*)msg.peer;
            }
            ObjectInfo& info=getInfomation(objectid.getKey());
            if(msg.peer==nullptr){
                info.status_=Ray::ObjectTableManager::ObjectInfo::READY;
                info.objSize_=0;
                info.usingCount_=0;
                info.obj_=nullptr;
                info.copies_.push_back(rayService_);
            }
            else if(msg.peer!=nullptr&&handle->size_>resultObjectMaxSize){
                RayService* service=(RayService*)msg.contentwo;
                info.status_=Ray::ObjectTableManager::ObjectInfo::READABLE;
                info.objSize_=handle->size_;
                info.usingCount_=0;
                info.obj_=nullptr;
                info.copies_.push_back(service);
            }
            else if(msg.peer!=nullptr&&handle->size_<=resultObjectMaxSize){
                addObjectToMemory(handle->size_);
                info.status_=Ray::ObjectTableManager::ObjectInfo::READY;
                info.objSize_=handle->size_;
                info.usingCount_=0;
                info.obj_=handle;
                info.copies_.push_back(rayService_);
            }
            getResult(objectid,handle);
            break;
    }
}

//已测
void Ray::ObjectTableManager::getResult(Ray::ObjectID id, Ray::ObjectHandle handle){
    ObjectInfo& info=getInfomation(id.getKey());
    auto end=info.DependencyCallbackQueue_.end();
    for(auto begin=info.DependencyCallbackQueue_.begin();begin!=end;begin++){
        (*begin)();
    }
    info.DependencyCallbackQueue_.clear();
}

//已测
void Ray::ObjectTableManager::receiveRemovedObjectnotification(ObjectKey& key){
    //先将内存上的空间腾出来
    ObjectInfo& info=getInfomation(key);
    if(info.status_==Ray::ObjectTableManager::ObjectInfo::READY){
        sharedObjectptr_->removeObject(info.objSize_);
    }
    auto index=objTable_.find(key);
    objTable_.erase(index);
}

//已测
void Ray::ObjectTableManager::receiveReadObjectInfoRequest(RayService* peerRayService, ObjectID objID){
    ObjectInfo& info=getInfomation(objID.getKey());
    if(info.status_==ObjectTableManager::ObjectInfo::ObjStatus::PENDING){
        info.DependencyCallbackQueue_.push_back([objID,this,info,peerRayService](){
            this->sendObjectInfo(objID,info.objSize_,peerRayService->getNodeID());
        });
    }
    else{
        //如果是READABLE, READY,SWAPEDOUT,info就一定有大小,因为这是向ObjID的Owner的节点请求，所以一定是在这个节点，你如果等于0，就表示没有找到
        sendObjectInfo(objID,info.objSize_,peerRayService->getNodeID());
    }
}

//已测
void Ray::ObjectTableManager::sendReadObjectInfoRequest(ObjectID objID){
    const uint32_t nodeid = objID.getKey().getOwnerNodeID();
    if(connectionsClient_.find(nodeid) == connectionsClient_.end()){
        startClient(nodeid,[this,objID](){
            this->sendReadObjectInfoRequest(objID);
        });
        return;
    }
    else{
        if(connectionsClient_.find(nodeid)->second.isConnect==false){
            connectionsClient_.find(nodeid)->second.calls.push_back([this,objID](){
                this->sendReadObjectInfoRequest(objID);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    //这里的msg.content要修改
    ObjectID* objid=new ObjectID(objID);
    Msg* msg=new Msg();
    msg->category=SendObjectInfo;
    msg->content=objid;
    msg->peer=rayService_;


    Message message;
    message.buff=msg;
    message.bufflength=8;
    message.size=256;
    node->write(connectionsClient_[nodeid].connection,message,nullptr);
    node->read(connectionsClient_[nodeid].connection,[this,node,nodeid](Message message){
        this->HandleMessage(message);
        node->read(connectionsClient_[nodeid].connection,[this,node,nodeid](Message message){
            this->HandleMessage(message);
        });
    });
}

//已测
void Ray::ObjectTableManager::sendObjectInfo(ObjectID objID, size_t objSize,NodeID nodeid){
    //ObjectInfo& information=getInfomation(objID.getKey());
    if(connectionsClient_.find(nodeid) == connectionsClient_.end()){
        startClient(nodeid,[this,objID,objSize,nodeid](){
            this->sendObjectInfo(objID,objSize,nodeid);
        });
        return;
    }
    else{
        if(connectionsClient_.find(nodeid)->second.isConnect==false){
            connectionsClient_.find(nodeid)->second.calls.push_back([this,objID,objSize,nodeid](){
                this->sendObjectInfo(objID,objSize,nodeid);
            });
            return;
        }
    }
    ObjectInfo* info=new ObjectInfo(getConstInfomation(objID.getKey()));
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    ObjectID* objid=new ObjectID(objID);
    
    Msg* msg=new Msg();
    msg->category=receiveCompleteObjectInfo;
    msg->content=objid;
    
    msg->peer=info;
    Message message;
    message.buff=msg;
    message.bufflength=sizeof(ObjectInfo);
    message.size=20000;
    node->write(connectionsClient_[nodeid].connection,message,nullptr);
}

//已测
void Ray::ObjectTableManager::receiveObjectInfo(ObjectID objID, size_t objSize){
    //将的到的info的消息进行修改
    ObjectInfo& info=getInfomation(objID.getKey());
    auto end=info.DependencyCallbackQueue_.end();
    for(auto begin=info.DependencyCallbackQueue_.begin();begin!=end;begin++){
        (*begin)();
    }
    info.DependencyCallbackQueue_.clear();
}

//已测
void Ray::ObjectTableManager::sendReadObjectRequest(ObjectID objID){
    //先找到一个拥有该对象的节点
    const uint32_t nodeid = objID.getKey().getOwnerNodeID();
    if(connectionsClient_.find(nodeid) == connectionsClient_.end()){
        startClient(objID.getKey().getOwnerNodeID(),[this,objID](){
            this->sendReadObjectRequest(objID);
        });
        return;
    }
    else{
        if(connectionsClient_.find(nodeid)->second.isConnect==false){
            connectionsClient_.find(nodeid)->second.calls.push_back([this,objID](){
                this->sendReadObjectRequest(objID);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    ObjectID* objid=new ObjectID(objID);
    Msg* msg=new Msg();
    msg->category=SendObject;
    msg->peer=rayService_;
    msg->content=objid;

    Message message;
    message.buff=msg;
    message.bufflength=8;
    message.size=256;

    node->write(connectionsClient_[nodeid].connection,message,nullptr);
    node->read(connectionsClient_[nodeid].connection,[this,node,nodeid](Message message){
        this->HandleMessage(message);
        node->read(connectionsClient_[nodeid].connection,[this,node,nodeid](Message message){
            this->HandleMessage(message);
        });
    });
}

//已测
void Ray::ObjectTableManager::sendReadObjectFromOtherNode(ObjectID objID,NodeID id){
    if(connectionsClient_.find(id) == connectionsClient_.end()){
        startClient(id,[this,objID,id](){
            this->sendReadObjectFromOtherNode(objID,id);
        });
        return;
    }
    else{
        if(connectionsClient_.find(id)->second.isConnect==false){
            connectionsClient_.find(id)->second.calls.push_back([this,objID,id](){
                this->sendReadObjectFromOtherNode(objID,id);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    Msg* msg=new Msg();
    ObjectID* objid=new ObjectID(objID);
    msg->category=SendObject;
    msg->peer=rayService_;
    msg->content=objid;

    Message message;
    message.buff=msg;
    message.bufflength=8;
    message.size=256;

    node->write(connectionsClient_[id].connection,message,nullptr);
    node->read(connectionsClient_[id].connection,[this,node,id](Message message){
        this->HandleMessage(message);
        node->read(connectionsClient_[id].connection,[this,node,id](Message message){
            this->HandleMessage(message);
        });
    });
}


void Ray::ObjectTableManager::receiveReadObjectRequest(RayService* peerRayService, ObjectID objID){
    ObjectInfo& info=getInfomation(objID.getKey());
    if(info.status_==ObjectTableManager::ObjectInfo::ObjStatus::SWAPEDOUT){
        NodeID id=peerRayService->getNodeID();
        auto CallBack=[info,id,this,objID,peerRayService]()mutable{
            info.usingCount_++;
            info.copies_.push_back(peerRayService);
            
            this->sendObject(objID,objID.getObj(),id);
        };
        info.ReadyCallbackQueue_.push_back(CallBack);
        //这里要触发从磁盘上读的操作
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        //定义一个DiskIOTask

        auto callBack=[objID,this,id]()mutable{
            ObjectInfo& info=getInfomation(objID.getKey());
            info.status_=ObjectTableManager::ObjectInfo::ObjStatus::READY;
            auto end=info.ReadyCallbackQueue_.end();
            auto begin=info.ReadyCallbackQueue_.begin();
            for(;begin!=end;begin++){
                (*begin)();
            }
            info.ReadyCallbackQueue_.erase(begin,end);
            this->addObjectToMemory(info.objSize_);
        };
        node->submitTask(DiskIOTask::Direction::READ,info.objSize_,callBack);
    }
    else if(info.status_==ObjectTableManager::ObjectInfo::ObjStatus::READABLE){
        //已测
        //要在对应的objectinfo的ReadyCallbackQueue_加入回调函数
        NodeID id=peerRayService->getNodeID();
        auto CallBack=[id,this,objID,peerRayService]()mutable{
            ObjectInfo& info=getInfomation(objID.getKey());
            info.usingCount_++;
            info.copies_.push_back(peerRayService);
            this->sendObject(objID,info.obj_,id);
        };
        info.ReadyCallbackQueue_.push_back(CallBack);
        //拿到有这个对象的节点id
        NodeID nodeid;
        for(size_t i=0;i<info.copies_.size();i++){
            if(info.copies_[i]->getNodeID()!=localNodeID_){
                nodeid=info.copies_[i]->getNodeID();
                break;
            }
        }
        //向对应的节点发送消息说需要Object
        this->sendReadObjectFromOtherNode(objID,nodeid);
    }
    else if(info.status_==ObjectTableManager::ObjectInfo::ObjStatus::READY){
        //已测
        info.usingCount_++;
        info.copies_.push_back(peerRayService);
        sendObject(objID,info.obj_,peerRayService->getNodeID());
    }
}

//已测
void Ray::ObjectTableManager::sendObject(ObjectID objID, ObjectHandle obj,NodeID nodeid){
    if(connectionsClient_.find(nodeid) == connectionsClient_.end()){
        startClient(nodeid,[this,objID,obj,nodeid](){
            this->sendObject(objID,obj,nodeid);
        });
        return;
    }
    else{
        if(connectionsClient_.find(nodeid)->second.isConnect==false){
            connectionsClient_.find(nodeid)->second.calls.push_back([this,objID,obj,nodeid](){
                this->sendObject(objID,obj,nodeid);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    Message message;
    Msg* msg=new Msg();
    ObjectID* objI=new ObjectID(objID);
    msg->category=receiveCompleteObject;
    msg->peer=objI;
    ObjectHandle* objectHandle=new ObjectHandle(obj);
    msg->content=objectHandle;
    message.size=obj->size_;
    
    message.buff=msg;
    message.bufflength=sizeof(ObjectInfo);
    

    node->write(connectionsClient_[nodeid].connection,message,nullptr);
}

//已测
void Ray::ObjectTableManager::receiveObject(ObjectID objID, ObjectHandle obj){
    //修改ObjectInfo的状态
    ObjectInfo& info=getInfomation(objID.getKey());
    info.status_=ObjectTableManager::ObjectInfo::ObjStatus::READY;
    info.usingCount_++;

    info.objSize_=obj->size_;
    info.obj_=obj;

    auto end=info.ReadyCallbackQueue_.end();
    for(auto begin=info.ReadyCallbackQueue_.begin();begin!=end;begin++){
        (*begin)();
    }
    info.ReadyCallbackQueue_.clear();
}


void Ray::ObjectTableManager:: waitUntilReady(Ray::ObjectID obj, ReadyCallback callback){
    ObjectInfo& info=getInfomation(obj.getKey());
    if(info.objSize_==0){
        info.usingCount_++;
        callback();
    }
    else if(info.status_ == Ray::ObjectTableManager::ObjectInfo::READY){
        //已测
        info.usingCount_++;
        callback();
    }
    else if(info.status_==Ray::ObjectTableManager::ObjectInfo::READABLE){
        info.ReadyCallbackQueue_.push_back(callback);
        NodeID nodeid=-1;
        for(size_t i=0;i<info.copies_.size();i++){
            if(info.copies_[i]->getNodeID()!=localNodeID_){
                nodeid=info.copies_[i]->getNodeID();
                break;
            }
        }
        if(nodeid==-1){
            this->sendReadObjectRequest(obj);
        }
        else{
            this->sendReadObjectFromOtherNode(obj,nodeid);
        }
        //向对应的节点发送消息说需要Object
        
    }
    else if(info.status_==Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT){
        if(obj.getKey().getOwnerNodeID()==localNodeID_&&info.objSize_==0){
            callback();
            return;
        }
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        //定义一个DiskIOTask
        auto callBack=[obj,this]()mutable{
            ObjectInfo& info=getInfomation(obj.getKey());
            this->addObjectToMemory(info.objSize_);
            info.status_=ObjectTableManager::ObjectInfo::ObjStatus::READY;
            info.usingCount_++;
            auto end=info.ReadyCallbackQueue_.end();
            for(auto begin=info.ReadyCallbackQueue_.begin();begin!=end;begin++){
                (*begin)();
            }
            info.ReadyCallbackQueue_.clear();
        };
        node->submitTask(DiskIOTask::Direction::READ,info.objSize_,callBack);
    }
}

//okok
void Ray::ObjectTableManager::waitUntilReadable(ObjectID obj, DependencyCallback callback){
    //首先需要在本节点查找ObjectInfo
    ObjectInfo& info=getInfomation(obj.getKey());
    if(info.objSize_==0&&info.status_==Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT){
        //这里会找到obj对应的NodeID将其对应的objInfo进行更新
        //会在本地的消息当中先给放一个callback，读到新的时候会合并
        //这里会生成一个新的info
        //这里可能出现已经被删除了
        ObjectKey key=obj.getKey();
        ObjectInfo info;
        info.isPrimary_=false;
        info.objSize_=0;
        info.usingCount_=0;
        info.status_=Ray::ObjectTableManager::ObjectInfo::PENDING;

        info.DependencyCallbackQueue_.push_back(callback);

        objTable_[key]=info;

        sendReadObjectInfoRequest(obj);
    }
    else{
        if(info.status_ == Ray::ObjectTableManager::ObjectInfo::READY||info.status_ == Ray::ObjectTableManager::ObjectInfo::READABLE
        ||info.status_ == Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT){
            EventLoop::getInstance().addCallBack(std::bind(callback));
            return;
        }
        else if(info.status_ == Ray::ObjectTableManager::ObjectInfo::PENDING){
            info.DependencyCallbackQueue_.push_back(callback);
        }
        // else if(info.status_ == Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT){
        //     //未测
        //     std::shared_ptr<Node> nodeLocal=NodeManager::getIntance().searchNode(localNodeID_);
        //     auto Call=[](){

        //     }
        //     nodeLocal->submitTask(DiskIOTask::Direction::READ,info.objSize_,callback);
        //     //未测
        // }
    }
}

u_int32_t Ray::ObjectTableManager::CreateObjectID(){
    return startObjectID++;
}

//已测
Ray::ObjectHandle Ray::ObjectTableManager::get(ObjectID objID)
{
    //在自己的表当中去查找
    ObjectKey key=objID.getKey();
    if(objTable_.find(key) != objTable_.end()){
        return objTable_.find(key)->second.obj_;
    }
    else{
        return nullptr;
    }
}

//已测
Ray::ObjectID Ray::ObjectTableManager::put(Ray::ObjectHandle obj){
    u_int32_t id=CreateObjectID();
    ObjectKey key(localNodeID_,id);
    ObjectInfo info;
    info.isPrimary_=false;
    info.obj_=obj;
    info.usingCount_=0;
    if(obj==nullptr){
        info.objSize_=0;
        info.status_=ObjectTableManager::ObjectInfo::PENDING;
    }
    else{
        info.objSize_=obj->size_;
        info.status_=ObjectTableManager::ObjectInfo::READY;
        addObjectToMemory(obj->size_);
    }
    std::shared_ptr<ObjectLifeTimeController> controller=std::make_shared<ObjectLifeTimeController>(key,rayService_);
    ObjectID objectid;
    objectid.setController(controller);
    objTable_.insert(std::pair<Ray::ObjectKey,Ray::ObjectTableManager::ObjectInfo>(key,info));
    return objectid;
}

//已测
void Ray::ObjectTableManager::reportResult(ObjectID objectID, ObjectHandle object){
    NodeID id=objectID.getKey().getOwnerNodeID();
    ObjectInfo& info=getInfomation(objectID.getKey());
    if(localNodeID_ == id){
        //就可以直接执行回调函数
        if(object!=nullptr){
            addObjectToMemory(object->size_);
        }

        info.obj_=object;
        if(object==nullptr){
            info.objSize_=0;
        }
        else{
            info.objSize_=object->size_;
        }
        info.usingCount_=0;

        info.status_=Ray::ObjectTableManager::ObjectInfo::READY;
        info.isPrimary_=true;
        
        auto end=info.DependencyCallbackQueue_.end();
        for(auto begin=info.DependencyCallbackQueue_.begin();begin!=end;begin++){
            (*begin)();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
        }
        info.DependencyCallbackQueue_.clear();
    }
    else{
        if(connectionsClient_.find(id) == connectionsClient_.end()){
            startClient(id,[this,objectID,object](){
                this->reportResult(objectID,object);
            });
            return;
        }
        else{
            if(connectionsClient_.find(id)->second.isConnect==false){
                connectionsClient_.find(id)->second.calls.push_back([this,objectID,object](){
                    this->reportResult(objectID,object);
                });
                return;
            }
        }
        ObjectID* objId=new ObjectID(objectID);
        Msg* msg=new Msg();
        Message message;
        //判断这个对象是否加入到本地
        if(object==nullptr){
            msg->peer=nullptr;
            message.size=0;
        }
        else if(object->size_>resultObjectMaxSize){
            
            //首先在本地创建一个info,并且将其添加到本地
            sharedObjectptr_->addObject(object->size_);//这个地方有疑惑，到底生成的对象放在那里
            ObjectInfo infomation;
            infomation.isPrimary_=true;
            //这里只是一个将要被释放的值
            infomation.obj_=object;
            
            infomation.status_=Ray::ObjectTableManager::ObjectInfo::READY;
            infomation.usingCount_=0;
            infomation.objSize_=object->size_;
            infomation.copies_.push_back(rayService_);
            objTable_.insert(std::pair<ObjectKey, ObjectInfo>(objectID.getKey(),infomation));

            ObjectHandle* handle=new ObjectHandle(object);
            msg->peer=handle;
            message.size=256;
            msg->contentwo=rayService_;
        }
        else if(object->size_<=resultObjectMaxSize){
            ObjectHandle* handle=new ObjectHandle(object);
            msg->peer=handle;
            message.size=object->size_;
        }
        
        
        //就要发送消息给远端
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        msg->category=SendResult;
        msg->content=objId;
    
        message.buff=msg;
        message.bufflength=1;
        
        node->write(connectionsClient_[id].connection,message,nullptr);
        node->read(connectionsClient_[id].connection,[this,node,id](Message message){
            this->HandleMessage(message);
            node->read(connectionsClient_[id].connection,[this,node,id](Message message){
                this->HandleMessage(message);
            });
        });
    }
}

void Ray::ObjectTableManager::forEach(std::function<bool (const ObjectKey&, ObjectInfo&)> call){
    auto begin=objTable_.begin();
    auto end=objTable_.end();
    for(;begin!=end;begin++){
        bool status=call(begin->first,begin->second);
        if(!status){
            return;
        }
    }
}

void Ray::ObjectTableManager::notifyNodeToRemoveCopy(const ObjectKey& key,NodeID id){
    if(connectionsClient_.find(id) == connectionsClient_.end()){
        startClient(id,[this,key,id](){
            this->notifyNodeToRemoveCopy(key,id);
        });
        return;
    }
    else{
        if(connectionsClient_.find(id)->second.isConnect==false){
            connectionsClient_.find(id)->second.calls.push_back([this,key,id](){
                this->notifyNodeToRemoveCopy(key,id);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    ObjectKey* ky=new ObjectKey(key);
    Msg* msg=new Msg();
    msg->category=ModifyCopies;
    msg->content=ky;
    msg->peer=rayService_;

    Message message;
    message.buff=msg;
    message.bufflength=1;
    message.size=1;
    node->write(connectionsClient_[id].connection,message,nullptr);
}

void Ray::ObjectTableManager::receiveCopyRemovedNotification(RayService* peerRayService,const ObjectKey& key){
    ObjectInfo& info=getInfomation(key);
    if(info.objSize_==0&&info.status_==ObjectInfo::SWAPEDOUT){
        return;
    }
    info.usingCount_--;
    auto begin=info.copies_.begin();
    auto end=info.copies_.end();
    for(;begin!=end;begin++){
        if((*begin)==peerRayService){
            begin=info.copies_.erase(begin);
            return;
        }
    } 
}

//这个是删除对象的Object
void Ray::ObjectTableManager::deleteKey(const ObjectKey& key){
    //这里要减少引用计数
    ObjectInfo& info=getInfomation(key);
    if(info.objSize_==0&&info.status_==Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT){
        return;
    }
    else{
        info.usingCount_--;
    }
    NodeID id=key.getOwnerNodeID();
    notifyNodeToRemoveCopy(key,id);
}

//已测
Ray::ObjectTableManager::ObjectInfo Ray::ObjectTableManager::getConstInfomation(const ObjectKey& key){
    if(objTable_.find(key)!=objTable_.end()){
        return objTable_.find(key)->second;
    }
    return INVALID_OBIECTINFOMATION;
}

//已测
Ray::ObjectTableManager::ObjectInfo& Ray::ObjectTableManager::getInfomation(const ObjectKey& key){
    if(objTable_.find(key)==objTable_.end()){
        return INVALID_OBIECTINFOMATION;
    }
    return objTable_.at(key);
}

void Ray::ObjectTableManager::sendRemovedNotification(ObjectKey& key,NodeID id){
    if(connectionsClient_.find(id) == connectionsClient_.end()){
        startClient(key.getOwnerNodeID(),[this,key,id]()mutable{
            this->sendRemovedNotification(key,id);
        });
        return;
    }
    else{
        if(connectionsClient_.find(id)->second.isConnect==false){
            connectionsClient_.find(id)->second.calls.push_back([this,key,id]()mutable{
                this->sendRemovedNotification(key,id);
            });
            return;
        }
    }
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    ObjectKey* ky=new ObjectKey(key);
    Msg* msg=new Msg();
    msg->category=DeleteObject;
    msg->content=ky;

    Message message;
    message.buff=msg;
    message.bufflength=1;
    message.size=1;
    node->write(connectionsClient_[id].connection,message,nullptr);
}

//这个是删除对象包括对象信息
void Ray::ObjectTableManager::deletObj(ObjectKey& key){
    ObjectInfo info=getInfomation(key);
    if(info.objSize_==0&&info.status_==ObjectInfo::SWAPEDOUT){
        return;
    }
    auto begin=info.copies_.begin();
    auto end=info.copies_.end();
    for(;begin!=end;begin++){
        NodeID id=(*begin)->getNodeID();
        sendRemovedNotification(key,id);
    }
    deleteKey(key);
}  

//已测
void Ray::ObjectTableManager::setSharedObjectStorage(const float memoryRatio,float memorySize){
    sharedObjectptr_=std::make_shared<SharedObjectStorage>(localNodeID_);
    sharedObjectptr_->setFreeMemorySize(memoryRatio*memorySize);
    sharedObjectptr_->setLocalSharedMemorySize(memoryRatio*memorySize);
    #ifndef TestStorage
    sharedObjectptr_->setObjectTableManager(this);
    #endif
}

//已测
void Ray::ObjectTableManager::exit(){
    //首先删除所有的ObjectID,就是删除clear的时候，ObjectInfo会在connections之前还是之后删除
    objTable_.clear();
    connectionsClient_.clear();
}

//已测
bool  Ray::ObjectTableManager::addObjectToMemory(size_t objSize){
    return sharedObjectptr_->addObject(objSize);
}

void Ray::ObjectTableManager::reduceObjectUsingCountInRayTask(Ray::RayTaskHandle task){
    size_t objectNumberInTask=task->inputDependency_.size();
    if(objectNumberInTask==0){
        return;
    }
    for(auto begin=task->inputDependency_.begin();begin!=task->inputDependency_.end();begin++){
        ObjectInfo& info=getInfomation(begin->getKey());
        if(info.objSize_==0&&info.status_==ObjectInfo::SWAPEDOUT){
            continue;
        }
        info.usingCount_--;
    }
}