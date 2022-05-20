#pragma once
#include "RayBaseDefination.h"
#include "Connection.h"
#include "Node.h"

namespace Ray
{
    class GlobalInformationServer
    {
        using TestGlobalData=std::function<void(GlobalResourceTable data)>;

        GlobalResourceTable globalTable_;
        Time broadcastCycle_;
        NodeID localNodeID_;
        std::vector<Connection> connections_;
        bool                    broadStatus_;
        bool                    acceptStatus;                
        

        void receiveLocalResourceInfo(NodeID nodeID, const LocalResourceTable& table);
        void broadcastGlobalResourceInfo();
        std::variant<bool,Connection> IsConnection(NodeID destID);
        
        void AcceptCall(std::shared_ptr<Node> node,Connection connection);
    public:
        TestGlobalData testdata;

        GlobalInformationServer();
        GlobalInformationServer(NodeID id);
        void push(Connection connect){
            connections_.push_back(connect);
        }
        //测试代码
        void testGlobalData(){
            testdata(globalTable_);
        }

        void setLocalNodeID(NodeID id){
            localNodeID_=id;
        }
        void setBroadStatus_(bool status){
            broadStatus_=status;
        }

        bool getStatus(){
            return broadStatus_;
        }
        void startbroadcastGlobalR();

        void setAcceptStatus(bool status){
            acceptStatus=status;
        }
        bool getAcceptStatus(){
            return acceptStatus;
        }

        int getSize(){
            return globalTable_.size();
        }
        int getConnectionSize(){
            return connections_.size();
        }
        void start();
        void close();
    };
}