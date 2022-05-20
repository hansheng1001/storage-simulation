#ifndef NETDRIVERCIRCLE_H
#define NETDRIVERCIRCLE_H

#include"Base.h"

struct NetDirverCircle{
    unsigned int m_senddelay;//传输延迟
    unsigned int m_internetdelay;//网络延迟
    BandWidth   m_downBandWidth;//上行带宽
    BandWidth   m_upBandWidth;//下行带宽
    
    unsigned int      m_sendcircleSize;
    unsigned int      m_recivecircleSize;

    NetDirverCircle(unsigned int senddelay,unsigned int internetdelay,BandWidth downBandWidth,BandWidth upBandWidth):
        m_senddelay(senddelay),
        m_internetdelay(internetdelay),
        m_downBandWidth(downBandWidth),
        m_upBandWidth(upBandWidth)    
    {
        m_sendcircleSize=m_upBandWidth*m_senddelay;
        m_recivecircleSize=m_downBandWidth*m_senddelay;
    }
    NetDirverCircle(const NetDirverCircle& cirlce)
    {
        m_senddelay=cirlce.m_senddelay;
        m_internetdelay=cirlce.m_internetdelay;
        m_upBandWidth=cirlce.m_upBandWidth;
        m_downBandWidth=cirlce.m_downBandWidth;

        m_sendcircleSize=cirlce.m_sendcircleSize;
        m_recivecircleSize=cirlce.m_recivecircleSize;
    }
    NetDirverCircle& operator=(const NetDirverCircle& cirlce){
        m_senddelay=cirlce.m_senddelay;
        m_internetdelay=cirlce.m_internetdelay;
        m_upBandWidth=cirlce.m_upBandWidth;
        m_downBandWidth=cirlce.m_downBandWidth;

        m_sendcircleSize=cirlce.m_sendcircleSize;
        m_recivecircleSize=cirlce.m_recivecircleSize;
        return (*this);
    }

    float getUnusedSendCircleSize()
    {
        return m_sendcircleSize;
    }

    float getUnusedReciveCircleSize()
    {
        return m_recivecircleSize;
    }

    float getTotalSendCircleSize()
    {
        return m_upBandWidth * m_senddelay;
    }

    float getTotalReciveCircleSize()
    {
        return m_downBandWidth * m_senddelay;
    }

};

#endif