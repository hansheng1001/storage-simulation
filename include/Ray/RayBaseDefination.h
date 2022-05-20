#pragma once
#include "../Base.h"
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Ray {
struct Object :public std::enable_shared_from_this<Object>{
    boost::any data_;
    size_t     size_;
    std::shared_ptr<Object> get_pointer(){
        return shared_from_this();
    }
};

using ObjectHandle = std::shared_ptr<Object>;

class ObjectKey {
    NodeID   ownerNodeID_;
    uint32_t perNodeObjID_;  //是指对象在Node中的对象编号？wanghs

public:
    ObjectKey(NodeID ownerNode, uint32_t IDInNode) : ownerNodeID_(ownerNode), perNodeObjID_(IDInNode) {}

    bool operator==(const ObjectKey& other) const {
        return ownerNodeID_ == other.ownerNodeID_ && perNodeObjID_ == other.perNodeObjID_;
    }

    const NodeID getOwnerNodeID() const {
        return ownerNodeID_;
    }

    const uint32_t getIDInNode() const {
        return perNodeObjID_;
    }

    size_t hash() const {
        return std::hash<NodeID>()(ownerNodeID_) ^ std::hash<uint32_t>()(perNodeObjID_);
    }
};

//无效key的定义
const ObjectKey INVALID_OBJKEY = ObjectKey(0, 0);

struct ObjectKeyHashFunc {
    size_t operator()(const ObjectKey& objKey) const {
        return objKey.hash();
    }
};

class RayService;
class ObjectID;

class ObjectLifeTimeController {
    ObjectKey   objKey_;
    RayService* service_;  // owner节点对应的RayService

public:
    ObjectLifeTimeController(ObjectKey key, RayService* service) : objKey_(key), service_(service) {}

    // get不能阻塞，当对象未产生或不在本地时，直接返回null
    //需要阻塞等待对象时，请提交一个新任务，并传递该对象的ObjectID作为新任务的输入参数
    ObjectHandle getObj(Ray::ObjectID id);

    ObjectKey getKey() const {
        return objKey_;
    }

    //析构函数需要从所有对象表中移除该对象的信息
    ~ObjectLifeTimeController();
};

class ObjectID {
    std::shared_ptr<ObjectLifeTimeController> controller_;

public:
    ObjectID(){}
    ObjectKey getKey() const {
        if (!controller_)
            return INVALID_OBJKEY;
        return controller_->getKey();
    }

    ObjectHandle getObj() {
        if (!controller_)
            return nullptr;
        return controller_->getObj(*this);
    }

    void setController(std::shared_ptr<ObjectLifeTimeController> controller) {
        controller_ = controller;
    }

    int getQuote(){
        return controller_.use_count();
    }
};

class ResourceSet {
public:
    std::unordered_map<uint32_t, float> resources_;
    ResourceSet() {}
    ResourceSet(const std::unordered_map<uint32_t, float>& resources) : resources_(std::move(resources)) {}
    float get(uint32_t resoruceID) {
        auto it = resources_.find(resoruceID);
        if (it == resources_.end())
            return 0;
        return it->second;
    }
};

using LocalResourceTable  = std::unordered_map<uint32_t, float>;
using GlobalResourceTable = std::unordered_map<NodeID, LocalResourceTable>;

struct RayEnv {
    RayService* service_;
    std::function<void(ObjectHandle)>
        reportFinish_;  //调用RayService::reportResult,还有就是TaskExecutor::whenTaskFinish
    //这里绑定的是有返回ObjectHandle，如果没有了就直接将reportFinish_==nullptr
    RayEnv() : service_(nullptr), reportFinish_(nullptr) {}
};

using RayEnvHandle = std::shared_ptr<RayEnv>;

struct RayTask:public std::enable_shared_from_this<RayTask>{
    size_t                            taskID_;
    NodeID                            startNodeID_;
    ObjectID                          returnValueObjID_;
    ResourceSet                       resourceRequest_;
    std::vector<ObjectID>             inputDependency_;
    std::function<void(RayEnvHandle)> taskFunc_;

    std::shared_ptr<RayTask> get_pointer(){
        return shared_from_this();
    }
};

    using RayTaskHandle = std::shared_ptr<RayTask>;

    class RayScheduler;
    class TaskExecutor;
    class ObjectTableManager;
};  // namespace Ray
