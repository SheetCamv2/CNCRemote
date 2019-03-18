/******************************************************************
EdingCNC server plugin
Copyright 2018 Stable Design <les@sheetcam.com>


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

#ifndef EDINGCNC_PLUGIN_H
#define EDINGCNC_PLUGIN_H
#include "cncserver.h"
#include "cncapi2.h"

using namespace std;
using namespace CncRemote;

enum CURSTATE {stateNEVER, stateIDLE, stateTOOLCHANGE, stateJOG, statePAUSED, stateRUNNING, stateANY};

class EdingCncServer : public CncRemote::Server
{
public:
    EdingCncServer();
    ~EdingCncServer();
	bool LoadDll();
    virtual void Poll();

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
	virtual vector<int> GetGCodes();
	virtual vector<int> GetMCodes();
	virtual bool StartPreview(const int recommendedSize);
	virtual PreviewData GetPreview();

protected:
	bool CheckPause();
	bool WaitMove();
private:

	CNCGETPAUSESTATUS CncGetPauseStatus;
	CNCGETSPINDLESTATUS CncGetSpindleStatus;
	CNCGETTOOLDATA CncGetToolData;
	CNCISSERVERCONNECTED CncIsServerConnected;
	CNCCONNECTSERVER CncConnectServer;
	CNCDISCONNECTSERVER CncDisConnectServer;
	CNCLOADJOB CncLoadJob;
	CNCLOADTOOLTABLE CncLoadToolTable;
	CNCUPDATETOOLDATA CncUpdateToolData;
	CNCGETSTATE CncGetState;
	CNCGETCURRENTT CncGetCurrentT;
	CNCGETSTATETEXT CncGetStateText;
	CNCGETCURINTERPRETERLINE CncGetCurInterpreterLine;
	CNCGETCUREXECLINE CncGetCurExecLine;
	CNCGETJOINTSTATUS CncGetJointStatus;
	CNCGETSIMULATIONMODE CncGetSimulationMode;
	CNCGETALLAXESHOMED CncGetAllAxesHomed;
	CNCGETSINGLESTEPMODE CncGetSingleStepMode;
	CNCGETBLOCKDELETE CncGetBlockDelete;
	CNCGETACTUALFEEDOVERRIDE CncGetActualFeedOverride;
	CNCGETACTUALFEED CncGetActualFeed;
	CNCGETWORKPOS CncGetWorkPos;
	CNCGETMACHINEPOS CncGetMachinePos;
	CNCGETIOSTATUS CncGetIOStatus;
	CNCGETCURRENTMCODESTEXT CncGetCurrentMcodesText;
	CNCGETCURRENTGCODESTEXT CncGetCurrentGcodesText;
	CNCRUNSINGLELINE CncRunSingleLine;
	CNCSTARTJOG CncStartJog;
	CNCSETOUTPUT CncSetOutput;
	CNCSETFEEDOVERRIDE CncSetFeedOverride;
	CNCREWINDJOB CncRewindJob;
	CNCRESET CncReset;
	CNCRESET2 CncReset2;
	CNCGETTRACKINGSTATUS  CncGetTrackingStatus;
	CNCPAUSEJOB CncPauseJob;
	CNCRUNORRESUMEJOB CncRunOrResumeJob;
	CNCSETSIMULATIONMODE CncSetSimulationMode;
	CNCSINGLESTEPMODE CncSingleStepMode;
	CNCENABLEBLOCKDELETE CncEnableBlockDelete;
	CNCSTOPJOG CncStopJog;
	CNCGETINPUT CncGetInput;
	CNCABORTJOB CncAbortJob;
	CNCGETEMSTOPACTIVE CncGetEMStopActive;
	CNCMOVETO CncMoveTo;
	CNCGETACTUALTOOLZOFFSET CncGetActualToolZOffset;
	CNCGETACTUALORIGINOFFSET CncGetActualOriginOffset;
	CNCSETTRACKINGVELOCITY CncSetTrackingVelocity;
	CNCSTARTVELOCITYTRACKING CncStartVelocityTracking;
	CNCSTOPTRACKING CncStopTracking;
	CNCGETJOINTCONFIG CncGetJointConfig;
	CNCSETSPEEDOVERRIDE CncSetSpeedOverride;
	CNCGETOPTIONALSTOP CncGetOptionalStop;
	CNCGETBLOCDELETE CncGetBlocDelete;
	CNCGETOUTPUT CncGetOutput;
	CNCINMILLIMETERMODE CncInMillimeterMode;
	CNCGETPROGRAMMEDFEED CncGetProgrammedFeed;
	CNCGETCURRENTTOOLNUMBER CncGetCurrentToolNumber;
	CNCGETCURRENTGCODESETTINGSTEXT CncGetCurrentGcodeSettingsText;
	CNCENABLEOPTIONALSTOP CncEnableOptionalStop;
	CNCLOGFIFOGET CncLogFifoGet;
	CNCGRAPHFIFOGET CncGraphFifoGet;
	CNCSTARTRENDERGRAPH CncStartRenderGraph;
	CNCGETMOTIONSTATUS CncGetMotionStatus;

	void GetPausePos(CNC_CART_DOUBLE *pos, int *spindleOutput, int *FloodOutput, int *mistOutput,
		int *lineNumber, int *valid, int* doArray, int *arrayX, int *arrayY);
	void PollConnected();
	bool GuiRunning();

	HMODULE m_cncDll;
	bool m_connected;
	int m_connectCount;
	bool m_jogging;
	double m_zMax;
	int m_recSize;
	Axes m_graphOffset;
};
#endif

