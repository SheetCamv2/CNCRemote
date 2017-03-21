
#ifndef LINUXCNC_H_INCLUDED
#define LINUXCNC_H_INCLUDED

#include "cncserver.h"

class LinuxCnc : public CncRemote::Server
{
public:
    LinuxCnc();
    void ConnectLCnc();
    bool Poll();
    virtual void UpdateState();


private:

    virtual void HandlePacket(const Packet & pkt);
    void SetMode(const int mode);
    int SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c);
    void SendJogStep(const int axis, const double val);

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
    bool m_connected;
    double m_maxSpeedLin;
    double m_maxSpeedAng;
};



#endif //#ifndef LINUXCNC_H_INCLUDED
