/****************************************************************
LinuxCNC server
Copyright 2017 Stable Design <les@sheetcam.com>


This program is free software; you can redistribute it and/or modify
it under the terms of the Mozilla Public License Version 2.0 or later
as published by the Mozilla foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License for more details.

You should have received a copy of the Mozilla Public License
along with this program; if not, you can obtain a copy from mozilla.org
******************************************************************/


#ifndef LINUXCNC_H_INCLUDED
#define LINUXCNC_H_INCLUDED

#include "cncserver.h"
#include "timer.h"
#include "rtapi.h"		// RTAPI realtime OS API
//#include "hal.h"		// HAL public API decls
//#include "../src/hal/hal_priv.h"	// private HAL decls

using namespace CncRemote;

class LinuxCnc : public CncRemote::Server
{
public:
    LinuxCnc();
    void ConnectLCnc();
    bool Poll();
    virtual void UpdateState();
    virtual Connection * CreateConnection(CActiveSocket * client, Server * server);
    static void ZeroJog();


/*    void SetMode(const int mode);
    inline void SendJog(const int axis, const double vel);
    int SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c);
    void SendJogStep(const int axis, const double val);
    hal_data_u * FindPin(const char * name, hal_type_t type);
    void LoadAxis(const int index);*/


private:

    int halId;

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
    bool m_connected;
//    UTimer m_jogTimer;

};



#endif //#ifndef LINUXCNC_H_INCLUDED
