
#ifndef LINUXCNC_H_INCLUDED
#define LINUXCNC_H_INCLUDED

#include "cncserver.h"
#include "timer.h"
#include "rtapi.h"		// RTAPI realtime OS API
#include "hal.h"		// HAL public API decls
#include "../src/hal/hal_priv.h"	// private HAL decls


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
    inline void SendJog(const int axis, const double vel);
    int SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c);
    void SendJogStep(const int axis, const double val);
    void ZeroJog();
    hal_data_u * FindPin(const char * name, hal_type_t type);
    void LoadAxis(const int index);


    struct JOGAXIS
    {
        hal_s32_t *counts;
        hal_bit_t *enable;
        hal_float_t *scale;
        hal_bit_t *velMode;
    };

    JOGAXIS m_halAxes[MAX_AXES];

    int halId;

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
    bool m_connected;
    double m_maxSpeedLin;
    double m_maxSpeedAng;
    UTimer m_jogTimer;

    double m_jogAxes[MAX_AXES];
};



#endif //#ifndef LINUXCNC_H_INCLUDED
