#include "Base.h"
#include "Connection.h"
#include "Node.h"

class PLayerServer
{
private:
    /* data */
    NodeID  localNodeID_;
    std::vector<Connection> connections_;
    bool status_;
public:
    void acceptMessage(Connection connection);
    PLayerServer(/* args */);
    ~PLayerServer();
    void init();
    void push_back(Connection connect){
        connections_.push_back(connect);
    }
    void receiveMessage();
    void handleMessage(Message);
};
