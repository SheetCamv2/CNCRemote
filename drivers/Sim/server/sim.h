#ifndef SIM_PLUGIN_H
#define SIM_PLUGIN_H

#include "cncserver.h"

using namespace std;

class Sim : public CncRemote::Server
{
public:
    virtual void HandlePacket(const Packet & pkt);
    virtual void UpdateState();
    bool Poll();

private:
    CncRemote::Axes m_jogVel;
};

#endif

