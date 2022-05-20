#include "Base.h"
#include "BaseTask.h"
#include "Connection.h"
#include "Callbacks.h"

class PLayerClient
{
private:
    /* data */
    NodeID  localNodeID_;
    NodeID  pLayerID_;
    bool    connectionStatus_;
    Connection connection_;
private:
    void startClient(CallBack call);  //网络发送和启动
public:
    PLayerClient(NodeID localID):localNodeID_(localID),pLayerID_(1),connectionStatus_(false){
    }
    ~PLayerClient();
    void submitNetInTask(NetInputTask,CallBack);
    void submitNetOutTask(NetInputTask,CallBack);
    void close();
    void setConnection(Connection connection);
    void setConnectionStataus(bool status);
};
