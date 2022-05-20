#include "Node.h"
#include "TCPController.h"
#include "Event.h"
#include "EventLoop.h"
#include "NodeManager.h"
#include "Callbacks.h"
#include "Base.h"
#include "ManagerLinkUnOrderMap.h"
#include "LinkManager.h"
#include"Connection.h"
#include"NetDriver.h"

Node::Node(){
}

Node::Node(const NodeID id):
    m_id(id)
{
    // std::shared_ptr<LinkManager> manager=std::make_shared<LinkManager>(id);
    // ManagerLinkUnOrderMap::getIntance()[id]=manager;

    // manager->setReplyConnect(std::bind(&LinkManager::ReplyConnect,manager,std::placeholders::_1));
}

Node::~Node(){
}

void Node::close(Connection conn){
    ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->close(conn);
}

void Node::setNetInfo(const NetDirverCircle& netInfo){
    m_netDriver->setNetInfo(netInfo);
}

void Node::setNodeID(NodeID id){
    m_id=id;
}

void Node::setComputationArch(const ComputationArch& arch)
{
    m_arch.NUMALatencyFactor_=arch.NUMALatencyFactor_;
    m_arch.memoryNodes_=arch.memoryNodes_;
    m_arch.sockets_=arch.sockets_;
}

void Node::setNetDriver(std::shared_ptr<NetDriver> netdriver){
    m_netDriver=netdriver;
}

void Node::addDisk(const Disk& disk)
{
    m_disks.push_back(disk);
}

void Node::initScheduler(ComputationTaskSchedulerInitFunc createScheduler)
{
    m_scheduler = createScheduler(m_arch);
}

void Node::initDiskDriver(DiskDriverInitFunc createDiskDriver)
{
    for(const Disk& disk : m_disks)
        m_diskDrivers.push_back(createDiskDriver(disk));
}

void Node::listen(){
    std::shared_ptr<LinkManager> link=ManagerLinkUnOrderMap::getIntance().searchLink(m_id);
    if(link==nullptr){
        std::cout << "link = nullptr" << m_id << std::endl;
    }
    link->listen();
}

void Node::listenObjectTable(){
    std::shared_ptr<LinkManager> link=ManagerLinkUnOrderMap::getIntance().searchLink(m_id);
    if(link==nullptr){
        std::cout << "link = nullptr" << m_id << std::endl;
    }
    link->listenObjectTable();
}

void Node::listenRayService(){
    std::shared_ptr<LinkManager> link=ManagerLinkUnOrderMap::getIntance().searchLink(m_id);
    if(link==nullptr){
        std::cout << "link = nullptr" << m_id << std::endl;
    }
    link->listenRayService();
}

bool Node::connect(Connection connection,ConnectBack call){
    ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->connect(connection,call);
    return true;
}

Connection Node::accept(ConnectBack call,int port){
    return ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->accept(call,port);
}

bool Node::write(Connection connection,Message message,CallBack call=nullptr){
    return ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->write(connection,message,call);
}

bool Node::read(Connection connection,ReadBack call){
    return ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->read(connection,call);
}

Port Node::CreatePort(){
    return ManagerLinkUnOrderMap::getIntance().searchLink(m_id)->CreatePort();
}

Connection Node::CreateConnection(NodeID sourceID,Port sourcePort,NodeID destID,Port destPort=0){
    return Connection(sourceID,sourcePort,destID,destPort);
}

void Node::addEventToSchedule(std::shared_ptr<Event> event){
    EventLoop::getInstance().AddEvent(event);
}

// Time Node::getInternetTime(DataSize size){
//     return m_internetCount->CountInternetTime(size,m_netInfo);
// }

const ComputationArchRuntimeState& Node::getComputationArchRuntimeState() const
{
    return m_scheduler->getState();   
}

const DiskRunTimeState& Node::getDiskRunTimeState(unsigned diskIndex) const
{
    return m_diskDrivers[diskIndex]->getState();
}

void Node::submitTask(DiskIOTask::Direction direction_,unsigned long IOLength_,CallBack call, int number){
    DiskIOTask task;
    task.direction_=direction_;
    task.IOLength_=IOLength_;
    m_diskDrivers[number]->submitDiskIOTask(task,call);
}

bool Node::allocMemory(size_t size){
    return m_scheduler->allocSharedMemory(size); 
}

NetDirverCircle Node::getNetDriverInfo() const
{
    return m_netDriver->getNetInfo();
}