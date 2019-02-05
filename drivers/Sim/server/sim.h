/****************************************************************
CNCRemote simulator
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

#ifndef SIM_PLUGIN_H
#define SIM_PLUGIN_H

#include "cncserver.h"

using namespace std;
using namespace CncRemote;


class Sim : public CncRemote::Server
{
public:
    Sim();
    bool Poll();

protected:
	virtual void UpdateState(State& state);
	virtual void DrivesOn(const bool state);
	virtual void JogVel(const Axes velocities);
	virtual void JogStep(const Axes distance, const double speed);
	virtual bool Mdi(const string line);
	virtual void SpindleOverride(const double percent);
	virtual void FeedOverride(const double percent);
	virtual void RapidOverride(const double percent);
	virtual bool LoadFile(const string file);
	virtual bool CloseFile();
	virtual void CycleStart();
	virtual void CycleStop();
	virtual void FeedHold(const bool state);
	virtual void BlockDelete(const bool state);
	virtual void SingleStep(const bool state);
	virtual void OptionalStop(const bool state);
	virtual void Home(const BoolAxes axes);
	virtual Axes GetOffset(const unsigned int index);



	struct MACHINESTATE
	{
		bool controlOn;
		bool paused;
		bool blockDelete;
		bool optStop;
		bool running;
		bool step;
		double feedOverride;
		int curLine;
		int busy;
		double spindleCmd;
		double spindleSpeed;
		double spindleOverride;
		CncRemote::Axes jogVel;
		CncRemote::Axes target;
		double feedRate;
		int runCount;
	} machine;

protected:
private:

};

#endif

