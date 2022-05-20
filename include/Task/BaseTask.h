typedef unsigned int Plog_ID;//这里的ID是指P层的ID

enum ResourceType { CPU, MEM, PLASMA_MEM };  // 这个不应该放在这里

struct NetInputTask{
    unsigned int Size_;
};

enum pMessageCategry{Write,Read};

struct PMessage
{
    /* data */
    pMessageCategry category;
    NetInputTask input;
};

const float    PlogSize             = 4.0;   // 一个Plog的大小
const uint32_t MetaPerPlog          = 1024;  // 一个Plog对应的元数据大小
const uint32_t MetaPerObj           = 1024;  // 一个HWObject对应的元数据大小
const float    GarbagePercent       = 0.3;   // 低利用率Plog占比
const float    UtilizationThreshold = 0.2;   // 低利用率阈值

const float    DupPercent           =0.3;    //每块元数据当中重复数据所占的比率