#ifndef CONNECTION_H
#define CONNECTION_H

#include<string>
#include<variant>
#include"Base.h"

enum connect{SYN,SYN_ACK,TIME_OUT};
enum ReceiveCategory{ACCEPT,CONNECT,RECEIVE,CLOSE};

struct Message{
  public:
  unsigned int   size;//这个表示只代表数据的大小
  int   bufflength;
  void* buff;//这个表示node要干什么
  
  Message(int s,int length,std::string word):
    size(s),bufflength(length)
  {}
  Message():
    size(-1),
    bufflength(-1),
    buff(nullptr)
  {}
};

struct Connection;
//typedef std::variant<bool,Connection> ReturnConnectionValue;

struct Connection{
  public:
  NodeID    m_sourceID;
  Port      m_sourcePort;
  NodeID    m_destID;
  Port      m_destPort;

  Connection()
  {}
  Connection(NodeID sourceID,Port sourcePort,NodeID destID,Port destPort):
    m_sourceID(sourceID),
    m_sourcePort(sourcePort),
    m_destID(destID),
    m_destPort(destPort)
  {}
  bool operator==(const Connection& conn){
    return (
      (this->m_destID==conn.m_destID)&&
      (this->m_sourceID==conn.m_sourceID)&&
      (this->m_sourcePort==conn.m_sourcePort)&&
      (this->m_destPort==conn.m_destPort)
    );
  }
  void swap(){
    NodeID  tempSourceID=m_sourceID;
    Port    tempSourcePort=m_sourcePort;
    m_sourceID=m_destID;
    m_sourcePort=m_destPort;

    m_destID=tempSourceID;
    m_destPort=tempSourcePort;
  }
};

struct ConnectionKeyHashFunc
{
    size_t operator()(const Connection &connect)const{
        return std::hash<NodeID>()(connect.m_destID);
    }
};

#endif


