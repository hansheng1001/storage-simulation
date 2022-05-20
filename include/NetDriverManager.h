#ifndef NETDRIVERMANAGER_H
#define NETDRIVERMANAGER_H

#include<unordered_map>
#include<memory>

class NetDriver;
class NetDriverManager
{
    static NetDriverManager _instance;
private:
    /* data */
    std::unordered_map<int,std::shared_ptr<NetDriver>> m_netDriverManager;
    NetDriverManager(/* args */);
public:
    ~NetDriverManager();
    static NetDriverManager& getIntance();
    size_t size();
    std::shared_ptr<NetDriver> searchNetDriver(int id);
    void insert(int,std::shared_ptr<NetDriver>);
    void deleteNetDriver(int id);
    void clear();
};


#endif