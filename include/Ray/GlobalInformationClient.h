#pragma once
#include "Connection.h"
#include "RayBaseDefination.h"
#include "Callbacks.h"

#include <functional>

namespace Ray {
class GlobalInformationClient {

    std::function<LocalResourceTable()>                       getLocalResourceCallback_;
    std::function<void(std::shared_ptr<GlobalResourceTable>)> updateGlobalResourceCallback_;
    NodeID                                                    localNodeID_;
    NodeID                                                    GCSNodeID_;
    Time                                                      heartbeatCycle_;
    bool                                                      sendStatus;

    Connection connection_;
    bool       isConnection_;

    void sendLocalResourceInfo();
    void receiveGlobalResourceInfo(std::shared_ptr<GlobalResourceTable> table);

    void setConnection(Connection connection);

public:


    GlobalInformationClient() : isConnection_(false) {}
    GlobalInformationClient(NodeID id, NodeID GCSNode);
    void setConnectionStatus(bool status);
    void setGetLocalResourceCallback(std::function<LocalResourceTable()> call);

    void setUpdateGlobalResourceCallback(std::function<void(std::shared_ptr<GlobalResourceTable>)> call);
    bool getStatus(){
        return sendStatus;
    }
    void setStatus(bool status){
        sendStatus=status;
    }
    void startClient(CallBack call);  //网络发送和启动
    void init();
    void startSendInfo();
    void close();
};
}  // namespace Ray