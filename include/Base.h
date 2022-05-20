
#ifndef BASE_H
#define BASE_H

#include<iostream>
#include<random>
#include<memory>
#include<string>
#include<variant>

typedef int NodeID;
typedef double BandWidth;
typedef float DiskSpace;
typedef float DiskIORate;
typedef float CpuFreq;
typedef float DataSize;
typedef uint64_t Time;

typedef float Delay;//网络延迟

typedef int Port;

typedef int relyNumber;

using objectID=std::pair<int,int>;

enum EventCategory{
    NOTIFYREAD,
    NOTIFYWRITE,
    HANDLEVENT,
};

enum Serverport
{
    /* data */
    GlobalInfomationPort,
    RayServicePort,
    ObjectTablePort 
};


class noncopyable{
    public:
    noncopyable(const noncopyable&);
    noncopyable& operator=(const noncopyable&);
    protected:
    noncopyable()=default;
    ~noncopyable()=default;
};

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
  return ptr.get();
}

#endif