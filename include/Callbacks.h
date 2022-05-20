#ifndef CALLBACKS_H
#define CALLBACKS_H

#include"Base.h"

#include<functional>
#include<string>
#include <utility>

class Connection;
class Message;

typedef std::function<void()> ITimeCallback;

// typedef std::function<void()> StratPlugin;
// typedef std::function<void()> StopPlugin;

typedef std::function<void()> CallBack;
typedef std::function<void(Message)> ReadBack;
typedef std::function<void(Connection& )> ConnectAtferBack;
typedef std::function<void(Connection)> ConnectBack;
typedef std::function<void(Connection)> AddConnection;

typedef bool Category;


#endif