
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

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
    bool m_connected;
};



#endif //#ifndef LINUXCNC_H_INCLUDED
