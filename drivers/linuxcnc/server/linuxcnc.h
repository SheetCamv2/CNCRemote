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
#include "rcs.hh"
#include "posemath.h"
#include "emc.hh"
#include "emc_nml.hh"
#include "nml_oi.hh"
#include "canon.hh"
#include "emcglb.h"
#include "emccfg.h"
#include "inifile.hh"
#include "rcs_print.hh"
#include "timer.hh"
#include "rtapi.h"

using namespace CncRemote;

enum LINEAR_UNIT_CONVERSION {
    LINEAR_UNITS_CUSTOM = 1,
    LINEAR_UNITS_AUTO,
    LINEAR_UNITS_MM,
    LINEAR_UNITS_INCH,
    LINEAR_UNITS_CM
};

#define INCH_PER_MM (1.0/25.4)
#define CM_PER_MM 0.1
#define GRAD_PER_DEG (100.0/90.0)
#define RAD_PER_DEG TO_RAD	// from posemath.h

#define JOGTELEOP 0
#define JOGJOINT  1

class LinuxCnc : public CncRemote::Server
{
public:
    LinuxCnc();
    void DisconnectNml();
    bool ConnectNml();
    bool OK();
    int IniLoad(const char *filename);
    bool ConnectLCnc();
    bool Poll();
    static void ZeroJog();
    bool CommandSend(RCS_CMD_MSG & cmd);
    bool SetMode(const EMC_TASK_MODE_ENUM mode);
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
    void *m_lastMessage;

    RCS_CMD_CHANNEL *m_emcCommandBuffer;
    RCS_STAT_CHANNEL *m_emcStatusBuffer;
    EMC_STAT *m_emcStatus;
    NML *m_emcErrorBuffer;
    LINEAR_UNIT_CONVERSION m_linearUnitConversion;

};



#endif //#ifndef LINUXCNC_H_INCLUDED
