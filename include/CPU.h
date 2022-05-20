#pragma once

struct CPU
{
    double cycle_;
    double CPI_;
    unsigned coreNum_;
    unsigned node_;
};

class CPUStatus
{
    unsigned idleCore_;

public:
    CPUStatus(unsigned coreNum):idleCore_(coreNum){};
    
    bool hasIdleCore() const {
        return idleCore_ > 0;
    }

    unsigned getIdleCoreNum() const {
        return idleCore_;
    }

    void assignTask(){
        --idleCore_;
    }

    void taskFinish(){
        ++idleCore_;
    }
};