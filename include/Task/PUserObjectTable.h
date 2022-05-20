#include<unordered_map>

#include "EventLoop.h"
#include "BaseTask.h"
#include "RayBaseDefination.h"

class PUserObjectTable
{
    using ReadObject=std::function<void (Ray::ObjectHandle)>; 
private:
    /* data */
    std::unordered_map<Plog_ID,Ray::Object> objectTable_;
private:

public:
    PUserObjectTable(/* args */);
    ~PUserObjectTable();
    void  getObject(size_t objSize,ReadObject call){
        Time time;
        Ray::Object obj;
        obj.size_=objSize;
        Ray::ObjectHandle handle=std::make_shared<Ray::Object>(obj);
        EventLoop::getInstance().callAfter(time,std::bind(call,handle));
    }
};
