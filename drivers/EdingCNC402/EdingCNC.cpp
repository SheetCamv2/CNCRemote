/*
EdingCNC server plugin
Copyright 2019 Stable Design <les@sheetcam.com>


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

#include "winsock2.h"
#include "cnccomms.h"
#include "EdingCNC.h"

#include <tlhelp32.h>
#define ASARRAYD(n) ((double *)&n)
#define ASARRAYB(n) ((int *)&n)

EdingCncServer::EdingCncServer()
{
    m_connected = false;
    m_connectCount = 0;
    m_cncDll = NULL;
	m_zMax = 0;
}

EdingCncServer::~EdingCncServer()
{
    if(m_cncDll) FreeLibrary(m_cncDll);
}


void EdingCncServer::GetPausePos(CNC_CART_DOUBLE *pos, int *spindleOutput, int *FloodOutput, int *mistOutput,
                           int *lineNumber, int *valid, int* doArray, int *arrayX, int *arrayY)
{
    CNC_PAUSE_STS* sts = CncGetPauseStatus();
    memcpy(pos, &sts->pausePosition, sizeof(CNC_CART_DOUBLE));
    *spindleOutput = sts->pauseSpindleIOValue;
    *FloodOutput = sts->pauseFloodIOValue;
    *mistOutput = sts->pauseMistIOValue;
    *lineNumber = sts->pausePositionLine;
    *valid = sts->pausePositionValid;
    *doArray = sts->pauseDoArray;
    *arrayX = sts->pauseArrayIndexX;
    *arrayY = sts->pauseArrayIndexY;
}


bool EdingCncServer::LoadDll()
{
    if(m_cncDll)
    {
        return true;
    }
    wstring path;
    HKEY key;
    wstring regPath = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    LONG ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &key);
    if(ret != ERROR_SUCCESS)
    {
        regPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &key);
    }
    if(ret != ERROR_SUCCESS) return false; //something has gone horribly wrong
    DWORD    subKeys=0;               // number of subkeys


    // Get the class name and the value count.
    ret = RegQueryInfoKey(
              key,                     // key handle
              NULL,                // buffer for class name
              NULL,           // size of class string
              NULL,                    // reserved
              &subKeys,               // number of subkeys
              NULL,            // longest subkey size
              NULL,            // longest class string
              NULL,                // number of values for this key
              NULL,            // longest value name
              NULL,         // longest value data
              NULL,   // security descriptor
              NULL);       // last write time


    if(!subKeys)
    {
        RegCloseKey(key);
        return false;
    }

#define MAX_KEY_LENGTH 256

    TCHAR    keyName[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    nameSize;                   // size of name string

    int dllVersion = 0;
    for (DWORD i=0; i<subKeys; i++)
    {
		nameSize = MAX_KEY_LENGTH;
        ret = RegEnumKeyEx(key, i,keyName, &nameSize,  NULL, NULL, NULL, NULL);
        if (ret == ERROR_SUCCESS)
        {
            if(wcsncmp(keyName,L"CNC4.0",6) != 0) continue;
            int ver = keyName[6] - L'0';
            if(ver < dllVersion) continue;
            DWORD    values=0;
            HKEY key2;
            ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, (regPath + L"\\" + keyName).c_str(), 0, KEY_READ, &key2);
            if(ret != ERROR_SUCCESS) continue;
            TCHAR keyData[MAX_KEY_LENGTH];
            DWORD dataSize = MAX_KEY_LENGTH;
            ret = RegQueryValueExW(key2, L"Publisher", 0, NULL, (LPBYTE)keyData, &dataSize);
            if(ret != ERROR_SUCCESS ||
                    wcscmp(keyData,L"EDING CNC B.V.") != 0)
            {
                RegCloseKey(key2);
                continue;
            }
			dataSize = MAX_KEY_LENGTH;
			ret = RegQueryValueExW(key2, L"InstallLocation", 0, NULL, (LPBYTE)keyData, &dataSize);
            if(ret == ERROR_SUCCESS)
            {
                dllVersion = ver;
                path = keyData;
            }
            RegCloseKey(key2);
if (dllVersion == 2) break;
        }
    }
    RegCloseKey(key);
    if(path.empty())
    {
        return false;
    }
	path += L"cncapi.dll";
	m_cncDll = LoadLibraryW(path.c_str());
    if(!m_cncDll)
    {
        return false;
    }

#define GETSYMBOL(type, f)     f = (type)GetProcAddress(m_cncDll,#f); if(!f) ok = false;
#define GETSYMBOL2(type, f, a) f = (type)GetProcAddress(m_cncDll,#a); if(!f) ok = false;


    bool ok = true;
    GETSYMBOL2(CNCGETCURRENTT, CncGetCurrentT, CncGetCurrentToolNumber);
    GETSYMBOL2(CNCGETBLOCKDELETE, CncGetBlockDelete, CncGetBlocDelete);
    GETSYMBOL2(CNCGETWORKPOS, CncGetWorkPos, CncGetWorkPosition);
    GETSYMBOL2(CNCGETMACHINEPOS, CncGetMachinePos, CncGetMachinePosition);

    GETSYMBOL(CNCGETPAUSESTATUS, CncGetPauseStatus);
    GETSYMBOL(CNCGETSPINDLESTATUS, CncGetSpindleStatus);

    GETSYMBOL(CNCSETTRACKINGVELOCITY, CncSetTrackingVelocity);
    GETSYMBOL(CNCSTARTVELOCITYTRACKING, CncStartVelocityTracking);
    GETSYMBOL(CNCSTOPTRACKING, CncStopTracking);
    GETSYMBOL(CNCGETJOINTCONFIG, CncGetJointConfig);

    GETSYMBOL(CNCGETTOOLDATA, CncGetToolData);
    GETSYMBOL(CNCISSERVERCONNECTED, CncIsServerConnected);
    GETSYMBOL(CNCCONNECTSERVER, CncConnectServer);
    GETSYMBOL(CNCDISCONNECTSERVER, CncDisConnectServer);
	if (dllVersion < 3)
	{
		GETSYMBOL(CNCLOADJOB, CncLoadJob);
		GETSYMBOL(CNCGETCURINTERPRETERLINE, CncGetCurInterpreterLine);
	}else
	{
		GETSYMBOL2(CNCLOADJOB, CncLoadJob, CncLoadJobW);
		GETSYMBOL2(CNCGETCURINTERPRETERLINE, CncGetCurInterpreterLine, CncGetCurInterpreterLineNumber);
	}
    GETSYMBOL(CNCLOADTOOLTABLE, CncLoadToolTable);
    GETSYMBOL(CNCUPDATETOOLDATA, CncUpdateToolData);
    GETSYMBOL(CNCGETSTATE, CncGetState);
    GETSYMBOL(CNCGETSTATETEXT, CncGetStateText);    
	GETSYMBOL(CNCGETCUREXECLINE, CncGetCurExecLine);
    GETSYMBOL(CNCGETJOINTSTATUS, CncGetJointStatus);
    GETSYMBOL(CNCGETSIMULATIONMODE, CncGetSimulationMode);
    GETSYMBOL(CNCGETALLAXESHOMED, CncGetAllAxesHomed);
    GETSYMBOL(CNCGETSINGLESTEPMODE, CncGetSingleStepMode);
    GETSYMBOL(CNCGETACTUALFEEDOVERRIDE, CncGetActualFeedOverride);
    GETSYMBOL(CNCGETACTUALFEED, CncGetActualFeed);
    GETSYMBOL(CNCGETIOSTATUS, CncGetIOStatus);
    GETSYMBOL(CNCGETCURRENTMCODESTEXT, CncGetCurrentMcodesText);
    GETSYMBOL(CNCGETCURRENTGCODESTEXT, CncGetCurrentGcodesText);
    GETSYMBOL(CNCRUNSINGLELINE, CncRunSingleLine);
    GETSYMBOL(CNCSTARTJOG, CncStartJog);
    GETSYMBOL(CNCSETOUTPUT, CncSetOutput);
    GETSYMBOL(CNCSETFEEDOVERRIDE, CncSetFeedOverride);
    GETSYMBOL(CNCREWINDJOB, CncRewindJob);
    GETSYMBOL(CNCRESET, CncReset);
    GETSYMBOL(CNCRESET2, CncReset2);
    GETSYMBOL(CNCGETTRACKINGSTATUS, CncGetTrackingStatus);
    GETSYMBOL(CNCPAUSEJOB, CncPauseJob);
    GETSYMBOL(CNCRUNORRESUMEJOB, CncRunOrResumeJob);
    GETSYMBOL(CNCSETSIMULATIONMODE, CncSetSimulationMode);
    GETSYMBOL(CNCSINGLESTEPMODE, CncSingleStepMode);
    GETSYMBOL(CNCENABLEBLOCKDELETE, CncEnableBlockDelete);
    GETSYMBOL(CNCSTOPJOG, CncStopJog);
    GETSYMBOL(CNCGETINPUT, CncGetInput);
    GETSYMBOL(CNCABORTJOB, CncAbortJob);
    GETSYMBOL(CNCGETEMSTOPACTIVE, CncGetEMStopActive);
    GETSYMBOL(CNCMOVETO, CncMoveTo);
    GETSYMBOL(CNCGETACTUALTOOLZOFFSET, CncGetActualToolZOffset);
    GETSYMBOL(CNCGETACTUALORIGINOFFSET, CncGetActualOriginOffset);
	GETSYMBOL(CNCSETSPEEDOVERRIDE, CncSetSpeedOverride);
	GETSYMBOL(CNCGETOPTIONALSTOP, CncGetOptionalStop);
	GETSYMBOL(CNCGETBLOCDELETE, CncGetBlocDelete);
	GETSYMBOL(CNCGETOUTPUT, CncGetOutput);
	GETSYMBOL(CNCINMILLIMETERMODE, CncInMillimeterMode);
	GETSYMBOL(CNCGETPROGRAMMEDFEED, CncGetProgrammedFeed);
	GETSYMBOL(CNCGETCURRENTTOOLNUMBER, CncGetCurrentToolNumber);
	GETSYMBOL(CNCGETCURRENTGCODESETTINGSTEXT, CncGetCurrentGcodeSettingsText);
	GETSYMBOL(CNCENABLEOPTIONALSTOP, CncEnableOptionalStop);
	GETSYMBOL(CNCLOGFIFOGET, CncLogFifoGet);
	GETSYMBOL(CNCGRAPHFIFOGET, CncGraphFifoGet);
	GETSYMBOL(CNCSTARTRENDERGRAPH, CncStartRenderGraph);
	GETSYMBOL(CNCGETMOTIONSTATUS, CncGetMotionStatus);
		
		
		
		 		

    if(!ok)
    {
        FreeLibrary(m_cncDll);
    }
    return ok;
}

void EdingCncServer::Poll()
{
	PollConnected();
	CNC_LOG_MESSAGE msg;
	CNC_RC ret = CncLogFifoGet(&msg);
	if(ret == CNC_RC_OK)
	{
		LogMessage(msg.text);
	}
}

void EdingCncServer::PollConnected()
{
    if(!GuiRunning())
    {
        m_connectCount = 0;
        return;
    }
    if(m_connected)
    {
        return;
    }
    if(m_connectCount < 4)
    {
        m_connectCount++;
        return;
    }
    if(!LoadDll())
    {
        return;
    }
    CNC_RC res = CncConnectServer("");
    if(res != CNC_RC_ALREADY_RUNS)
    {
        /*		if(CncIsServerConnected())
        		{
        			CncDisConnectServer();
        		}*/
        return;
    }
    if(!CncIsServerConnected())
    {
        return;
    }
    m_connected = true;
/*    for(int ct=0; ct< CNC_MAX_TOOLS; ct++)
    {
        m_tools[ct] = CncGetToolData(ct);
    }*/
}

bool EdingCncServer::GuiRunning()
{
    PROCESSENTRY32 procEntry;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        return false ;
    }
    procEntry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnap, &procEntry))
    {
        CloseHandle(hSnap);
        return false ;
    }
    int found = 0;

    do
    {
        if (_tcsicmp(procEntry.szExeFile, TEXT("usbcnc.exe")) == 0 ||
                _tcsicmp(procEntry.szExeFile, TEXT("cnc.exe")) == 0)
        {
            found |= 1;
        }
        if (_tcsicmp(procEntry.szExeFile, TEXT("CncServer.exe")) == 0)
        {
            found |= 2;
        }
        if(found == 3)
        {
            CloseHandle(hSnap);
            return(true);
        }

    }
    while (Process32Next(hSnap, &procEntry));

    CloseHandle(hSnap);
    return(false);
}



void EdingCncServer::UpdateState(State& state)
{

	CNC_IE_STATE cnc = CncGetState();

	switch (cnc)
	{
	case CNC_IE_POWERUP_STATE:
	case CNC_IE_IDLE_STATE:
		state.machineState = mcOFFLINE;
		return;

	case CNC_IE_READY_STATE:
		if (!CncGetOutput(CNC_IOID_DRIVE_ENABLE_OUT))
		{
			state.machineState = mcOFF;
			break;
		}
		state.machineState = mcIDLE;
		break;

	case CNC_IE_EXEC_ERROR_STATE:
	case CNC_IE_INT_ERROR_STATE:
	case CNC_IE_ABORTED_STATE:
		state.machineState = mcOFF;
		break;

	case CNC_IE_RUNNING_JOB_STATE:
	case CNC_IE_RUNNING_LINE_STATE:
	case CNC_IE_RUNNING_SUB_STATE:
	case CNC_IE_RUNNING_SUB_SEARCH_STATE:
	case CNC_IE_RUNNING_LINE_SEARCH_STATE:
	case CNC_IE_PAUSED_LINE_STATE:
	case CNC_IE_PAUSED_JOB_STATE:
	case CNC_IE_PAUSED_SUB_STATE:
	case CNC_IE_PAUSED_LINE_SEARCH_STATE:
	case CNC_IE_PAUSED_SUB_SEARCH_STATE:
	case CNC_IE_RUNNING_LINE_HANDWHEEL_STATE:
	case CNC_IE_RUNNING_LINE_PAUSED_STATE:
		state.machineState = mcRUNNING;
		break;

	case CNC_IE_RUNNING_HANDWHEEL_STATE:
	case CNC_IE_RUNNING_AXISJOG_STATE:
	case CNC_IE_RUNNING_IPJOG_STATE:
		state.machineState = mcMOVING;
		break;

	case CNC_IE_RENDERING_GRAPH_STATE:
	case CNC_IE_SEARCHING_STATE:
	case CNC_IE_SEARCHED_DONE_STATE:
		state.machineState = mcMDI; //not strictly correct but need to indicate interpreter is busy
		break;
	}
	CNC_CART_DOUBLE absPos = CncGetMachinePos();

	CNC_MOTION_STATUS motStatus = CncGetMotionStatus();





	CNC_CART_DOUBLE workPos = CncGetWorkPos();
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		state.machinePos.array[ct] = ASARRAYD(absPos)[ct];
		state.position.array[ct] = ASARRAYD(workPos)[ct];
	}
	if (state.machinePos.z > m_zMax)
	{
		m_zMax = state.machinePos.z;
	}

	state.feedHold = CncGetPauseStatus()->pauseManualActionRequired;
	state.feedOverride = CncGetActualFeedOverride();
	state.optionalStop = CncGetOptionalStop();
	state.blockDelete = CncGetBlocDelete();
	state.currentLine = CncGetCurInterpreterLine();
	state.singleStep = CncGetSingleStepMode();
	CNC_SPINDLE_STS* spin = CncGetSpindleStatus();
	state.spindleCmd = spin->programmedSpindleSpeed;
	state.spindleActual = spin->actualSpindleSpeedSigned;
	if (spin->spindleIsOn)
	{
		if (spin->spindleDirection) state.spindleState = spinREV;
		else state.spindleState = spinFWD;
	}
	else
	{
		state.spindleState = spinOFF;
	}
	state.spindleOverride = spin->speedOverrideFactor;

	state.mist = CncGetOutput(CNC_IOID_COOLANT2_OUT);
	state.flood = CncGetOutput(CNC_IOID_COOLANT1_OUT);
	
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		CNC_JOINT_STS * joint = CncGetJointStatus(ct);
		state.homed.array[ct] = joint->isHomed;
	}

	double feed = 0;
	for (int ct = 0; ct < 3; ct++)
	{
		CNC_JOINT_CFG * joint = CncGetJointConfig(ct);
		if (joint)
		{
			feed += joint->maxVelocity * joint->maxVelocity;
		}
	}
	state.maxFeedLin = sqrt(feed);
	if (CncInMillimeterMode())
		state.gcodeUnits = 1;
	else
		state.gcodeUnits = 1/25.4;
	state.rapidOverride = 1;
	state.feedCmd = CncGetProgrammedFeed();
	state.feedActual = CncGetActualFeed();
	state.tool = CncGetCurrentToolNumber();

	char buf[80];
	CncGetCurrentGcodeSettingsText(buf);
	state.interpState = buf;
}

void EdingCncServer::DrivesOn(const bool drivesOn)
{
	ThreadLock lock = GetLock();
	if (CncGetEMStopActive()) //can't do anything if external estop active
	{
		return;
	}

	CNC_IE_STATE cnc = CncGetState();

	if (!drivesOn)
	{
		switch (cnc)
		{
		case CNC_IE_RUNNING_JOB_STATE:
		case CNC_IE_RUNNING_LINE_STATE:
		case CNC_IE_RUNNING_SUB_STATE:
		case CNC_IE_RUNNING_LINE_SEARCH_STATE:
		case CNC_IE_RUNNING_SUB_SEARCH_STATE:
			//Running state, Pause and reset, return
			CncPauseJob();
			break;

		case CNC_IE_RUNNING_AXISJOG_STATE:
		case CNC_IE_RUNNING_IPJOG_STATE:
			//These states cannot be reset
			CncStopJog(-1);
		}
//		CncAbortJob();
		CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 0);
		return;
	}

	if (cnc >= CNC_IE_READY_STATE && CncGetOutput(CNC_IOID_DRIVE_ENABLE_OUT) &&
		(cnc < CNC_IE_EXEC_ERROR_STATE || cnc >  CNC_IE_ABORTED_STATE)) return; //drives already on
	CncReset2(1);
	CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 1);


	//Check handwheel mode from server status and force back to non tracking mode if needed.
	CNC_TRACKING_STATUS trackSts = *CncGetTrackingStatus();
	bool serverHandwheelMode = false;

	//Update tracking if hand wheel
	if (trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_POS || trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_VEL)
	{
		serverHandwheelMode = true;
		CncStopTracking();
	}
}

void EdingCncServer::JogVel(const Axes velocities)
{

	CNC_IE_STATE cnc = CncGetState();
	
	bool wasMoving = cnc == CNC_IE_RUNNING_AXISJOG_STATE ||
		cnc == CNC_IE_RUNNING_HANDWHEEL_STATE;
	bool move = false;
	for(int ct=0; ct< CNC_MAX_AXES; ct++)
	{
		if (velocities.array[ct] != 0)
		{
			move = true;
			break;
		}
	}

	LockedState state = GetState();

	if(move)
	{
		if(CncStartVelocityTracking) //velocity tracking mode is quicker to respond than jog mode
		{
			if(!wasMoving)
			{
				CncStartVelocityTracking();
			}
			CNC_CART_BOOL move;
			CNC_CART_DOUBLE vels;
			for(int ct=0; ct< MAX_AXES; ct++)
			{
				ASARRAYB(move)[ct] = true;
				ASARRAYD(vels)[ct] = velocities.array[ct] * state->maxFeedLin;
			}
			CncSetTrackingVelocity(vels, move);
		}else //does not support tracking mode
		{
			double vel = 0;
			for (int ct = 0; ct < MAX_AXES; ct++)
			{
				double d = velocities.array[ct];
				vel += d * d;
			}
			vel = sqrt(vel) * state->maxFeedLin;
			CncStartJog((double *)velocities.array, vel, true);
		}
	}else
	{
		if (!wasMoving) return;
		if(CncStartVelocityTracking)
		{
			CncStopTracking();
		}else
		{
			CncStopJog(-1);
		}
	}
}

void EdingCncServer::JogStep(const Axes distance, const double speed)
{
	if (speed <= 0) return;
	CNC_IE_STATE cnc = CncGetState();
	if (cnc <= CNC_IE_ABORTED_STATE) return; //already moving so ignore

	LockedState state = GetState();
	CncStartJog((double *)distance.array, speed * state->maxFeedLin, false);
}


bool EdingCncServer::Mdi(const string line)
{
	return CncRunSingleLine((char *)line.c_str()) == CNC_RC_OK;
}

void EdingCncServer::SpindleOverride(const double percent)
{
	CncSetSpeedOverride(percent);
}

void EdingCncServer::FeedOverride(const double percent)
{
	CncSetFeedOverride(percent);
}

void EdingCncServer::RapidOverride(const double percent)
{
	LogMessage("Rapid override not supported");
}

bool EdingCncServer::LoadFile(const string file)
{
	ThreadLock lock = GetLock();
	CncString s1 = from_utf8(file.c_str());
	const wchar_t * s2 = s1.c_str();
	CNC_RC ret = CncLoadJob(s2);
//	return CncLoadJob(from_utf8(file.c_str()).c_str()) == CNC_RC_OK;
	return ret == CNC_RC_OK;
}

bool EdingCncServer::CloseFile()
{
	return CncLoadJob(L"") == CNC_RC_OK;
}

void EdingCncServer::CycleStart()
{
	switch(CncGetState())
	{
	case CNC_IE_PAUSED_LINE_STATE:        /* single line paused, by pause command */
	case CNC_IE_PAUSED_JOB_STATE:         /* paused running job, by pause command */
	case CNC_IE_PAUSED_SUB_STATE:         /* paused running sub , by pause command */
	case CNC_IE_PAUSED_LINE_SEARCH_STATE: /* paused running line search line running from search*/
	case CNC_IE_PAUSED_SUB_SEARCH_STATE:  /* paused running sub search subroutine running from search */
		if(!CheckPause())
		{
			return;
		}
		break;

	default:
		m_zMax = -1e17;
	}
	CncRunOrResumeJob();
}

void EdingCncServer::CycleStop()
{
	CncPauseJob();
	CncReset();
}

void EdingCncServer::FeedHold(const bool state)
{
	CncPauseJob();
}

void EdingCncServer::BlockDelete(const bool state)
{
	CncEnableBlockDelete(state);
}

void EdingCncServer::SingleStep(const bool state)
{
	CncSingleStepMode(state);
}

void EdingCncServer::OptionalStop(const bool state)
{
	CncEnableOptionalStop(state);
}

void EdingCncServer::Home(const BoolAxes axes)
{
	bool found = true;
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		if (!axes.array[ct])
		{
			found = false;
			break;
		}
	}
	if (found)
	{
		CncRunSingleLine("gosub home_all");
		return;
	}
	if (axes.z) CncRunSingleLine("gosub home_z");
	if (axes.x) CncRunSingleLine("gosub home_x");
	if (axes.y) CncRunSingleLine("gosub home_y");
	if (axes.a) CncRunSingleLine("gosub home_a");
	if (axes.b) CncRunSingleLine("gosub home_b");
	if (axes.c) CncRunSingleLine("gosub home_c");
}

Axes EdingCncServer::GetOffset(const unsigned int index)
{
	CNC_MOTION_STATUS motStatus = CncGetMotionStatus();
	CNC_OFFSET_AND_PLANE& plane = motStatus.activeOffsetAndPlane;

	double * ptr = NULL;
	int axes = MAX_AXES;
	switch (index)
	{
	case 0:
		axes = 3;
		ptr = (double *)&plane.toolOffset;
		break;

	case 1:
		ptr = (double *)&plane.currentG5X;
		break;

	case 2:
		ptr = (double *)&plane.g92Offset;
		break;

	case 3:
		ptr = (double *)&plane.spindleConfigOffset;
		break;

	case 4:
		ptr = (double *)&plane.totalOffset;

	default:
		axes = 0;
	}

	Axes ret;
	for (int ct = 0; ct < axes; ct++)
	{
		ret.array[ct] = *ptr++;
	}
	for (int ct = axes; ct < MAX_AXES; ct++)
	{
		ret.array[ct] = 0;
	}
	return ret;
}

vector<int> EdingCncServer::GetGCodes()
{
	vector<int> ret;
	char buf[80];
	CncGetCurrentGcodesText(buf);
	return ret;
}

vector<int> EdingCncServer::GetMCodes()
{
	vector<int> ret;
//	CncGetCurrentMcodesText(buf);
	return ret;
}



bool EdingCncServer::StartPreview(const int recommendedSize)
{
	m_recSize = recommendedSize;
	memset(&m_graphOffset, 0, sizeof(m_graphOffset));
	return CncStartRenderGraph(0,1) == CNC_RC_OK;
}

PreviewData EdingCncServer::GetPreview()
{
	PreviewData ret;
	CNC_GRAPH_FIFO_DATA point;
	for (int ct = 0; ct < m_recSize && CncGraphFifoGet(&point) == CNC_RC_OK; ct++)
	{
		PreviewAxes a;
		switch (point.type)
		{
		case CNC_MOVE_TYPE_G1:
		case CNC_MOVE_TYPE_PROBE:
			a.type = prevMOVE;
			break;

		case CNC_MOVE_TYPE_G0:
			a.type = prevRAPID;
			break;

		case CNC_MOVE_TYPE_START_POSITION:
			a.type = prevSTART;
			break;

		case CNC_MOVE_TYPE_ORIGIN_OFFSET:
			for (int ct = 0; ct < MAX_AXES; ct++)
			{
				m_graphOffset.array[ct] = ASARRAYD(point.pos)[ct];
			}
			continue;

		default:
			continue;
		}
		for (int ct = 0; ct < MAX_AXES; ct++)
		{
			a.array[ct] = ASARRAYD(point.pos)[ct] + m_graphOffset.array[ct];
		}
		ret.push_back(a);
	}
	return ret;
}
















/*
int EdingCncServer::OnNotify(const int index,const double val)
{
    if(index >= dllCONTROL_CHG)
    {
        ControlChanged((index - dllCONTROL_CHG) % 10000, val);
        return(0);
    }

    switch(index)
    {
    case dllTIMER:
        DoTimer();
        break;

    case dllENABLE:
        m_autoLoad = (int)val;
        break;

    case dllCONTROL_NUM:
        if(m_controlCount >= MAX_CONTROLS)
        {
            return(-1);
        }
        {
            CONTROL& ctrl = m_controls[m_controlCount++];
            ctrl.enabled = true;
            ctrl.value = 1e17;
            ctrl.number = val;
            ctrl.enState = stateANY;
            const ENABLE * ptr = g_enables;
            while(ptr->button > 0)
            {
                if(ptr->button == ((int)val) % 10000)
                {
                    ctrl.enState = ptr->enable;
                    break;
                }
                ptr++;
            }
        }
        break;

    case dllMDI:
    {
        scChar buf [1024];
        if(scPlugin->GetSetting(_T("MDIText"), buf, 1024) && buf[0] !=0)
        {
            wstring txt(buf);
            MDI(txt.ToAscii());
        }
    }
    break;

    }
    return(0);
}

void EdingCncServer::SendString(const int index, const wstring value)
{
    CONTROL * ptr = m_controls;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        if(index == ptr->number)
        {
            scPlugin->SetSetting(_T("stringData"), value.c_str());
            scPlugin->NotifyLua(dllSTRING, ct);
        }
        ptr++;
    }
}

void EdingCncServer::StatusText(const wstring& msg)
{
    SendString(funcSTATUS, msg);
}


void EdingCncServer::CheckDro(const int index, const double value)
{
    CONTROL * ptr = m_controls;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        if(index == ptr->number)
        {
            if(value != ptr->value)
            {
                scPlugin->NotifyLua(ct + dllCONTROL_CHG, value);
                ptr->value = value;
                switch(index)
                {
                case btnHome:
                    if(!value)
                    {
                        StatusText(_T("Not homed."));
                    }
                    break;
                }
            }
        }
        ptr++;
    }
}

bool EdingCncServer::OnPostBegin(const scChar * fileName, const bool local)
{
    if(local || !m_autoLoad || !CncIsServerConnected())
    {
        return(false);
    }

    CncLoadJob(_T(""));
    return(false);
}

void EdingCncServer::PopulateTool(CNC_TOOL_DATA& data, RoundTool * tool)
{
    data.id = tool->number;
    wstring name = tool->name;
    name.Truncate(79);
    strcpy(data.description,name.ToAscii());
    data.diameter = tool->diameter;
    data.orientation = 9;
    data.xOffset = 0;
    RotaryTool * rot = CASTROTARYTOOL(tool);
    if(rot)
    {
        data.zOffset = rot->lengthOffset;
    }
    else
    {
        data.zOffset = 0;
    }
}

void EdingCncServer::SyncTools()
{
    Tools& tools = Parts::Get().tools;
    const wxCharBuffer unused = wstring(_T("Unused")).ToAscii();
    for(int ct=1; ct< CNC_MAX_TOOLS; ct++)
    {
        bool found = false;
        for(int t=0; t< tools.GetCount(); t++)
        {
            RoundTool * rTool = CASTROUNDTOOL(tools[t]);
            if(rTool && rTool->number == ct)
            {
                found = true;
                break;
            }
        }
        if(found)
        {
            continue;
        }
        if(strcmp(m_tools[ct].description, unused) != 0)
        {
            strcpy(m_tools[ct].description,unused);
            CncUpdateToolData(&m_tools[ct], ct);
        }
    }
    for(int ct=0; ct < tools.GetCount(); ct++)
    {
        ToolBase* tool = tools[ct];
        RoundTool * rTool = CASTROUNDTOOL(tool);
        int num = rTool->number;
        if(rTool && num < CNC_MAX_TOOLS && num > 0)
        {
            CNC_TOOL_DATA data;
            PopulateTool(data, rTool);
            if(memcmp(&data, &m_tools[num], sizeof(CNC_TOOL_DATA)) != 0)
            {
                CncUpdateToolData(&data, num);
                m_tools[num] = data;
            }
        }
    }
    CncLoadToolTable();
}


void EdingCncServer::OnPostEnd(const scChar * fileName, const bool local)
{

    if(local || !m_autoLoad || fileName[0] ==0 || !CncIsServerConnected())
    {
        return;
    }

    if(CncGetState() != CNC_IE_READY_STATE)
    {
        wxMessageBox(_T("Eding CNC is busy. Code not loaded"));
        return;
    }


    CncLoadJob(fileName);
    if(m_autoSync)
    {
        SyncTools();
    }
}


void EdingCncServer::ScanTools()
{
    Tools& tools = Parts::Get().tools;
    m_toolCount++;
    if(m_toolCount >= tools.GetCount())
    {
        m_toolCount = 0;
    }
    RoundTool * tool = CASTROUNDTOOL(tools[m_toolCount]);
    if(tool)
    {
        int idx = tool->number;
        if(idx > 0 && idx < CNC_MAX_TOOLS)
        {
            CNC_TOOL_DATA src = CncGetToolData(idx);
            CNC_TOOL_DATA& dest = m_tools[idx];
            int dirty = 0;
            if(memcmp(&src, &dest, sizeof(CNC_TOOL_DATA)) != 0)
            {
                if(dest.zOffset != src.zOffset)
                {
                    RotaryTool * rTool = CASTROTARYTOOL(tool);
                    if(rTool)
                    {
                        rTool->lengthOffset = src.zOffset;
                    }
                }
                if(dest.diameter != src.diameter)
                {
                    tool->diameter = src.diameter;
                    dirty = tool->diameter.GetDirtyFlags();
                }
                memcpy(&dest, &src, sizeof(CNC_TOOL_DATA));
            }
            if(dirty)
            {
                Parts::Get().SetDirty(dirty | DIRTY_FILE | DIRTY_TOOLS);
            }
        }
    }
}

//enum CURSTATE {stateNEVER, stateIDLE, statePAUSED, stateRUNNING, stateJOG, stateANY};
void EdingCncServer::EnableControls()
{
    static CNC_IE_STATE prevState = CNC_IE_LAST_STATE;
    CURSTATE cur = stateJOG;
    CNC_IE_STATE state = CncGetState();
    if(state == prevState)
    {
        return;
    }
    prevState = state;
    switch(state)
    {
    case CNC_IE_EXEC_ERROR_STATE:
    case CNC_IE_INT_ERROR_STATE:
    case CNC_IE_ABORTED_STATE:
    case CNC_IE_RUNNING_JOB_STATE:
        StatusText(wstring::FromAscii(CncGetStateText(state)));
        break;

    }
    if(state < CNC_IE_RUNNING_JOB_STATE)
    {
        cur = stateIDLE;
    }
    else if (state < CNC_IE_PAUSED_LINE_STATE)
    {
        cur = stateRUNNING;
    }

    else if (state < CNC_IE_RUNNING_HANDWHEEL_STATE)
    {
        CNC_PAUSE_STS* sts = CncGetPauseStatus();
        if(m_curState != statePAUSED &&
                sts->pauseManualActionRequired)
        {
            int tool = CncGetCurrentT();
            Tools& tools = Parts::Get().tools;
            wstring toolTxt = wstring::FromAscii(m_tools[tool].description);
            ToolBase * tl = tools.GetTool(tool);
            if(tl)
            {
                toolTxt = tl->GetName(false);
            }
            StatusText(wstring::Format(_T("Load tool %i: %s"),tool, toolTxt.c_str()));
        }
        cur = statePAUSED;
    }


    if( cur == m_curState)
    {
        return;
    }
    m_curState = cur;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        CONTROL& ctrl = m_controls[ct];
        bool en = cur <= ctrl.enState;
        if(en != ctrl.enabled)
        {
            ctrl.enabled = en;
            if(en)
            {
                scPlugin->NotifyLua(dllENABLECTRL, ct);
            }
            else
            {
                scPlugin->NotifyLua(dllDISABLECTRL, ct);
            }
        }
    }
}

void EdingCncServer::DoTimer(void)
{
    PollConnected();
    if(!m_connected)
    {
        return;
    }

    CheckDro(droJogPercent, m_jogVel * 100); //these two only really need to fire once...
    CheckDro(droStepSize, m_stepSize);
    CheckDro(btnAutoSync, m_autoSync);
    CheckDro(btnMonitorTools, m_autoMonitor);

    EnableControls();

    char buf[80];
    buf[0] = 0;
    CncGetCurrentGcodesText(buf);
    wstring strg = wstring::FromAscii(buf);
    CncGetCurrentMcodesText(buf);
    strg += _T("\n") + wstring::FromAscii(buf);
    if(strg != m_activeCodes)
    {
        m_activeCodes = strg;
        SendString(funcGMCODES, m_activeCodes);
    }

    for(int ct=0; ct< CNC_IOID_LAST; ct++)
    {
        CNC_IO_PORT_STS* s = CncGetIOStatus((CNC_IO_ID)ct);
        if(!s)
        {
            continue;
        }
        CheckDro(ct, s->lvalue);
        if(ct == CNC_IOID_TOOL_OUT)
        {
            CheckDro(btnSpindle, s->lvalue);
        }
    }

    CNC_CART_DOUBLE pos = CncGetMachinePos();
    double *posPtr = (double *) &pos;
    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CheckDro(ct + droMachineX, *posPtr);
        if(ct == 2) //Z axis
        {
            if(*posPtr > m_zMax)
            {
                m_zMax = *posPtr;
            }
        }
        posPtr++;
    }


    pos = CncGetWorkPos();
    posPtr = (double *) &pos;

    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CheckDro(ct + droX, *posPtr);
        posPtr++;
    }

    CheckDro(droActualFeed, CncGetActualFeed());

    CNC_SPINDLE_STS* sts = CncGetSpindleStatus();

    CheckDro(droSpindle, sts->actualSpindleSpeedSigned);

    CheckDro(droFRO, CncGetActualFeedOverride() * 100);

    CheckDro(btnBlockDelete, CncGetBlockDelete());
    CheckDro(btnSingleStep, CncGetSingleStepMode());
    CheckDro(btnHome, CncGetAllAxesHomed());
    CheckDro(btnSimulate, CncGetSimulationMode());

    posPtr = (double *) &pos;
    bool changed = false;
    for(int ct=0; ct < DRAW_AXES; ct++)
    {
        if(*posPtr != m_drawPos[ct])
        {
            changed = true;
            m_drawPos[ct] = *posPtr;
        }
        posPtr++;
    }

    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CNC_JOINT_STS * joint = CncGetJointStatus(ct);
        CheckDro(ct + btnHomeX, joint->isHomed);
        posPtr++;
    }

    if(changed)
    {
        scPlugin->GlRedraw();
    }
    int line = CncGetCurExecLine() - 1;
    if(line < 0)
    {
        line = CncGetCurInterpreterLine() - 1;
    }
    if(line != m_oldLine)
    {
        m_oldLine = line;
        scPlugin->NotifyLua(dllLINENUMBER, line);
    }
    if(m_autoMonitor)
    {
        ScanTools();
    }
}

void EdingCncServer::OnDrawDisplay(void)
{
    if(!m_displayOn || m_drawPos[0] == 1e17)
    {
        return;
    }
    glPushMatrix();
    glTranslatef(m_drawPos[0], m_drawPos[1] ,m_drawPos[2]);
    glColor3f(0.2,0.3,0.3);
    glBegin(GL_LINES);
    glVertex3f(-100000,0,0);
    glVertex3f(100000,0,0);
    glVertex3f(0,-100000,0);
    glVertex3f(0,100000,0);
    glEnd();
    glPopMatrix();
}

void EdingCncServer::MDI(const char * string)
{
    CncRunSingleLine((char *)string);
}

void EdingCncServer::MDI(const char * string, const double value)
{
    char buf[1024];
    sprintf(buf,string,value);
    CncRunSingleLine(buf);
}


#define ASARRAYD(n) ((double *)&n)
#define ASARRAYB(n) ((int *)&n)

void EdingCncServer::JogAxis(int button, bool run)
{
    int axis = button - btnJogXP;
    if(axis < 0)
    {
        return;
    }
    bool neg = axis & 1;
    axis /=2;
    if(axis >= CNC_MAX_AXES)
    {
        return;
    }
    double dir = neg ? -1 : 1;
    double vel = m_jogVel;
    if(m_fastJog)
    {
        vel = 1;
    }

    bool wasMoving = false;
    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        if(ASARRAYD(m_jogging)[ct] != 0)
        {
            wasMoving = true;
        }
    }

    if(run)
    {
        double dir = neg ? -1 : 1;
        if(CncStartVelocityTracking && ! m_step)
        {
            CNC_JOINT_CFG* cfg = CncGetJointConfig(axis);
            ASARRAYD(m_jogging)[axis] = (vel * dir) * cfg->maxVelocity;
            if(!wasMoving)
            {
                CncStartVelocityTracking();
            }
            CNC_CART_BOOL move;
            for(int ct=0; ct< CNC_MAX_AXES; ct++)
            {
                ASARRAYB(move)[ct] = true;
            }
            CncSetTrackingVelocity(m_jogging, move);
        }
        else
        {
            ASARRAYD(m_jogging)[axis] = m_stepSize * dir;
            CncStartJog(ASARRAYD(m_jogging), vel, !m_step);
        }
    }
    else
    {
        ASARRAYD(m_jogging)[axis] = 0;
        if(CncStartVelocityTracking && ! m_step)
        {
            CNC_CART_BOOL move;
            bool moving = false;
            for(int ct=0; ct< CNC_MAX_AXES; ct++)
            {
                moving |= ASARRAYD(m_jogging)[ct] != 0;
                ASARRAYB(move)[ct] = true;
            }
            if(moving)
            {
                CncSetTrackingVelocity(m_jogging, move);
                wxLogMessage(_T("x %f y %f"), m_jogging.x, m_jogging.y);
            }
            else
            {
                CncStopTracking();
            }
        }
        else
        {
            CncStopJog(axis);
        }

    }
}


void EdingCncServer::Toggle(CNC_IO_ID pin)
{
    CNC_IO_PORT_STS* s = CncGetIOStatus(pin);
    CncSetOutput(pin, !s->lvalue);
}

void EdingCncServer::DrivesOnOff()
{
    StatusText(_T(""));
    CNC_IE_STATE state = CncGetState();
    if (CncGetEMStopActive())
    {
        return;
    }
    switch (state)
    {
    case CNC_IE_POWERUP_STATE:
    case CNC_IE_IDLE_STATE:
//		 return;

    case CNC_IE_EXEC_ERROR_STATE:
    case CNC_IE_INT_ERROR_STATE:
    case CNC_IE_RENDERING_GRAPH_STATE:
    case CNC_IE_PAUSED_LINE_STATE:
    case CNC_IE_PAUSED_JOB_STATE:
    case CNC_IE_ABORTED_STATE:
        //These are all exceptional states, reset and done
        CncReset2(1);
        break;

    case CNC_IE_RUNNING_HANDWHEEL_STATE:
    case CNC_IE_RUNNING_LINE_HANDWHEEL_STATE:
    case CNC_IE_PAUSED_SUB_SEARCH_STATE:
    case CNC_IE_PAUSED_LINE_SEARCH_STATE:
    case CNC_IE_SEARCHED_DONE_STATE:
    case CNC_IE_READY_STATE:
        //These are all exceptional states, reset and done
//		CncReset2(0);
        CncAbortJob();
        CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 0);
        return;
        break;

    case CNC_IE_RUNNING_JOB_STATE:
    case CNC_IE_RUNNING_LINE_STATE:
    case CNC_IE_RUNNING_SUB_STATE:
    case CNC_IE_RUNNING_LINE_SEARCH_STATE:
    case CNC_IE_RUNNING_SUB_SEARCH_STATE:
        //Running state, Pause and reset, return
        CncPauseJob();
        CncReset2(1);
        break;

    case CNC_IE_RUNNING_AXISJOG_STATE:
    case CNC_IE_RUNNING_IPJOG_STATE:
        //These states cannot be reset
        CncStopJog(-1);
        CncReset2(1);
        break;

    default:
        break;
    }

    CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 1);

    //If we reach here, the state was CNC_IE_READY_STATE
    //Do the reset

    //Check handwheel mode from server status and force back to non tracking mode if needed.
    CNC_TRACKING_STATUS trackSts = *CncGetTrackingStatus();
    bool serverHandwheelMode = false;

    //Update tracking if hand wheel
    if (trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_POS || trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_VEL)
    {
        serverHandwheelMode = true;
        CncStopTracking();
    }


//    ChangeJogMode(CNC_JOGMODE_CONT, m_curHandwheelAxis, true);

    if (CncGetInput(CNC_IOID_DRIVE_ENABLE_OUT) == 1)
    {
        if (CncGetAllAxesHomed())
        {
            StatusText(_("Drives enabled."));
        }
        else
        {
            wstring msg = _("Drives enabled.");
            msg += _T(" ");
            msg += _("Not homed.");
            StatusText(msg);
        }
    }
    else
    {
        StatusText(_("Drives NOT enabled."));
    }
}

*/

bool EdingCncServer::CheckPause()
{

	CNC_PAUSE_STS* sts = CncGetPauseStatus();
	if(sts->pauseManualActionRequired)
	{
		return(true);
	}
	CNC_CART_DOUBLE pausePos;
	int spindle;
	int flood;
	int mist;
	int line;
	int valid;
	int doArray[CNC_MAX_AXES];
	int arrayX[CNC_MAX_AXES];
	int arrayY[CNC_MAX_AXES];
	GetPausePos(&pausePos, &spindle, &flood, &mist, &line, &valid, doArray, arrayX, arrayY);
	if(!valid)
	{
		return(false);
	}

	CNC_CART_DOUBLE offset = CncGetActualOriginOffset();
	offset.z += CncGetActualToolZOffset();

	static CNC_CART_DOUBLE machinePos;
	machinePos = CncGetMachinePos();
	if(memcmp(&pausePos, &machinePos, sizeof(CNC_CART_DOUBLE)) == 0)
	{
		return(true);
	}

	if(m_zMax < machinePos.z)
	{
		m_zMax = machinePos.z;
	}
	if(m_zMax < pausePos.z)
	{
		m_zMax = pausePos.z;
	}
/*
	wstring msg;
	if(machinePos.x != pausePos.x ||
		machinePos.y != pausePos.y)
	{
		msg += wstring::Format(_("Z to %.3f\n"), m_zMax - offset.z);
		msg += wstring::Format(_("X,Y to %.3f, %.3f\n"), pausePos.x - offset.x, pausePos.y - offset.y);
	}
	msg += wstring::Format(_("Z to %.3f\n"), pausePos.z - offset.z);
	msg = _("The machine move sequence will be:\n") + msg;

	int ans = wxMessageBox(msg, _("Safety move"), wxOK | wxCANCEL | wxICON_QUESTION);
	if(ans != wxOK)
	{
		return(false);
	}
*/
	CNC_CART_BOOL moveArray;

	for(int ct=0; ct < CNC_MAX_AXES; ct++)
	{
		ASARRAYB(moveArray)[ct] = 1;
	}
	//={1,1,1,1,1,1};

	if(machinePos.x != pausePos.x ||
		machinePos.y != pausePos.y)
	{
		machinePos.z = m_zMax;
		CncMoveTo(machinePos, moveArray, 0.2);
	}
	if(!WaitMove())
	{
		return(false);
	}
	CncSetOutput(CNC_IOID_COOLANT2_OUT, mist);
	CncSetOutput(CNC_IOID_COOLANT1_OUT, flood);
	CncSetOutput(CNC_IOID_TOOL_OUT, spindle);
	machinePos.x = pausePos.x;
	machinePos.y = pausePos.y;
	machinePos.a = pausePos.a;
	machinePos.b = pausePos.b;
	machinePos.c = pausePos.c;
	CncMoveTo(machinePos, moveArray, 0.2);
	if(!WaitMove())
	{
		return(false);
	}
	machinePos.z = pausePos.z;
	CncMoveTo(machinePos, moveArray, 0.2);
	if(!WaitMove())
	{
		return(false);
	}
	return(true);
}

bool EdingCncServer::WaitMove()
{
	CNC_IE_STATE state;
	do
	{
		Sleep(50);
		state = CncGetState();
	}
	while(state == CNC_IE_RUNNING_AXISJOG_STATE);
	return(state == CNC_IE_PAUSED_JOB_STATE);
}
