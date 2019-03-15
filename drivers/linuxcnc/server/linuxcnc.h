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
    static void ZeroJog();
    void SetMode(const int mode);
    void SendJog(const int axis, const double vel);
    int SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c);
    void SendJogStep(const int axis, const double val);

	virtual void UpdateState(State& state);
	virtual void DrivesOn(const bool state);
	virtual void JogVel(const Axes velocities);
	virtual void JogStep(const Axes distance, const double speed);
	virtual bool Mdi(const string line);
	virtual void SpindleOverride(const double percent);
	virtual void FeedOverride(const double percent);
	virtual void RapidOverride(const double percent);
	virtual bool LoadFile(const string fileName);
	virtual bool CloseFile();
	virtual void CycleStart();
	virtual void CycleStop();
	virtual void FeedHold(const bool state);
	virtual void BlockDelete(const bool state);
	virtual void SingleStep(const bool state);
	virtual void OptionalStop(const bool state);
	virtual void Home(const BoolAxes axes);
	virtual Axes GetOffset(const unsigned int index);
    virtual vector<int> GetGCodes();
    virtual vector<int> GetMCodes();




private:

    int halId;

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
    bool m_connected;

};



#endif //#ifndef LINUXCNC_H_INCLUDED
