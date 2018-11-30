//=====================================================================
//
//	Plugin.cpp - the optional custom part of the plugin
//
//	this source file can be filled with the actual custom code that
//	makes the plugin work. it is the choice of the developer to enable
//	which functions will be used from the available MachDevice calls.
//
//	if this is a mixed mode dll each function can be declared as either
//	an unmanaged or managed function.
//
//	please see the documentation in Plugin.h for the #define statements
//	that control each functions compilation.
//
//	if this is a mixed mode dll and you need to keep global managed
//	reference please see the MG class in ManagedGlobal.h
//
//	please read the notes and comments in MachDevice.cpp for general
//	information and disclaimers.
//
//=====================================================================

#include "stdafx.h"
#include "Plugin.h"
//#include "MachDevice.h"
#include "MachServer.h"


//---------------------------------------------------------------------
//	data area
//---------------------------------------------------------------------

MachServer* server = NULL;


#if 0

enum {jogNONE, jogON, jogOFF};
int g_jogStatus = jogNONE;

bool g_shutdown = false;
uintptr_t g_droHandle = 0;
HANDLE g_hmapFile = NULL;
int g_isMoving = 0;

#define MOVING_COUNT 2

void HandleCommand(CMDDATA& command, const bool mainThread);
void Ack(CMDTYPE command, HANDLE hPipe);
void DoJog(const JOGCMD& command, const bool perc);
void DoMDI(const CMDBUFFER& buf);



void DroThread(void * param)
{
	while(!g_shutdown && g_mappedStatus)
	{
		bool moving = false;
		for(int ct=0; ct< MAX_AXES; ct++)
		{
			g_mappedStatus->axes[ct] = (Engine->Axis[ct].Index / g_axes[ct].scale) - g_axes[ct].offset;
			moving |= Engine->Axis[ct].Jogging;
		}		
		moving |= Engine->TrajHead != Engine->TrajIndex;
		if(moving)
		{
			g_isMoving = MOVING_COUNT;
		}

/*		if(moving != wasMoving)
		{
			if(!moving)
			{
				g_mappedStatus->moving = false;
				g_isMoving = 0;
			}
			wasMoving = moving;
		}*/
		Sleep(5);
	}
	_endthread();
}


void HandleCommand(CMDDATA& command, const bool mainThread)
{
	static bool needSync[cmdMAX]={
		0, //cmdUNKNOWN
		0, //cmdFULL
		0, //cmdERROR
		0, //cmdSIZE
		0, //cmdJOG
		1, //cmdSETDRO
		0, //cmdGETDRO
		1, //cmdSETLED
		0, //cmdGETLED
		1, //cmdDOBUTTON
		1, //cmdMDI
	};

	HANDLE hPipe = command.pipe;
	int type = command.data.cmd.type;
	if(type >= cmdMAX)
	{
		Ack(cmdUNKNOWN, hPipe);
		return;
	}
	if(command.data.cmd.length != cmdSizes[type])
	{
		Ack(cmdSIZE, hPipe);
		return;
	}
	if(!mainThread && needSync[type])
	{
		if(!g_cmdData.Add(command))
		{
			Ack(cmdFULL, hPipe);
		}
		return;
	}
	switch(type)
	{
	case cmdJOG:
		DoJog((JOGCMD&)command.data, false);
		break;

	case cmdJOGPERC:
		DoJog((JOGCMD&)command.data, true);
		break;

	case cmdSETDRO:
		SetDRO(((DROCMD&)command.data).dro, ((DROCMD&)command.data).value);
		break;

	case cmdGETDRO:
		((DROCMD&)command.data).value = GetDRO(((DROCMD&)command.data).dro);
		SendCommand(command.data, hPipe);
		break;

	case cmdSETLED:
		SetLED(((LEDCMD&)command.data).led, ((LEDCMD&)command.data).value);
		break;

	case cmdGETLED:
		((LEDCMD&)command.data).value = GetLED(((LEDCMD&)command.data).led);
		SendCommand(command.data, hPipe);
		break;

	case cmdDOBUTTON:
		DoButton(((BTNCMD&)command.data).button);
		break;

	case cmdMDI:
		DoMDI(command.data);
		break;
	}
}

void Ack(CMDTYPE command, HANDLE hPipe)
{
	CMDBUFFER buf;
	buf.cmd.length = sizeof(CMDHDR);
	buf.cmd.type = command;
	SendCommand(buf,hPipe);
}

void DoJog(const JOGCMD& command, const bool perc)
{
	double linScale = 2048;
	double accScale = 2048 / (double)(Engine->PulseCount * 2);

	bool moving = false;

	for(int ct=0; ct<MAX_AXES; ct++)
	{
		AXISDATA& axis = g_axes[ct];
		double cmd;
		if(perc)
		{
			cmd = (command.axes[ct] * MainPlanner->Velocities[ct]) / 100;
		}else
		{
			cmd = command.axes[ct] / 60;
		}
		if(cmd < 0)
		{
			if(cmd < -MainPlanner->Velocities[ct])
			{
				cmd = -MainPlanner->Velocities[ct];
			}
		}else
		{
			if(cmd > MainPlanner->Velocities[ct])
			{
				cmd = MainPlanner->Velocities[ct];
			}
		}

		axis.acc = (int)(MainPlanner->Acceleration[ct] * axis.scale * accScale); 

		int jogAmount = (int)(cmd * axis.scale * linScale);
		axis.targetJog = jogAmount;
		if(jogAmount != 0)
		{
			moving = true;
			axis.jogging = true;
			g_isMoving = MOVING_COUNT;
		}
	}
	if(moving)
	{
		g_jogStatus = jogON;
	}else
	{
		g_jogStatus = jogOFF;
	}
}

void DoMDI(const CMDBUFFER& buf)
{
/*	wchar_t data[256];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, data, 256, buf.data, 256);*/
	g_isMoving = MOVING_COUNT;
	Code((LPCSTR)buf.data);
}

#endif

//---------------------------------------------------------------------
//
//	piInitControl() - Plugin extension of InitControl()
//
//		XML file can NOT be accessed since SetProName hasn't
//		been called yet
//
//		called EVEN if plugin is disabled
//
//---------------------------------------------------------------------

#ifdef PI_INITCONTROL
#ifdef _MANAGED
#pragma PI_MIX_INITCONTROL
#endif
bool piInitControl()
{
	return true;
}
#endif

//---------------------------------------------------------------------
//
//	piSetProName() - Plugin extension of SetProName()
//
//		XML file CAN be accessed
//
//		called EVEN if plugin is disabled
//
//---------------------------------------------------------------------

#ifdef PI_SETPRONAME
#ifdef _MANAGED
#pragma PI_MIX_SETPRONAME
#endif
char* piSetProName(LPCSTR name)
{
	return "CncRemote";
}
#endif

//---------------------------------------------------------------------
//
//	piPostInitControl() - Plugin extension of PostInitControl()
//
//		XML file can NOT be accessed
//
//		called ONLY if plugin is enabled
//
//---------------------------------------------------------------------

#ifdef PI_POSTINITCONTROL
#ifdef _MANAGED
#pragma PI_MIX_POSTINITCONTROL
#endif
void piPostInitControl()
{
	server = new MachServer;
	if (server->Bind() != CncRemote::errOK)
	{
		delete server;
		server = NULL;
		return;
	}
	server->StartLoop();
}
#endif

//---------------------------------------------------------------------
//
//	piConfig() - Plugin extension of Config()
//
//		called if user presses CONFIG in Config|Config Plugins
//		even if plugin is disabled
//
//		XML file CAN be accessed
//
//---------------------------------------------------------------------

#ifdef PI_CONFIG
#ifdef _MANAGED
#pragma PI_MIX_CONFIG
#endif
void piConfig()
{
}
#endif

//---------------------------------------------------------------------
//
//	piStopPlug() - Plugin extension of StopPlug()
//
//---------------------------------------------------------------------

#ifdef PI_STOPPLUG
#ifdef _MANAGED
#pragma PI_MIX_STOPPLUG
#endif
void piStopPlug()
{
	delete server;
}
#endif

//---------------------------------------------------------------------
//
//	piUpdate() - Plugin extension of Update()
//
//		XML file can NOT be accessed
//
//		called ONLY if plugin is enabled
//
//		WARNING - when you enable a plugin it immediately is added
//		to the update loop. if you haven't initialized some items
//		because PostInitControl() hasn't been called you can get
//		some problems!!!
//
//---------------------------------------------------------------------

#ifdef PI_UPDATE
#ifdef _MANAGED
#pragma PI_MIX_UPDATE
#endif
void piUpdate()
{
	if(server) server->Poll();

/*
	CMDDATA cmd;
	double* orgPtr = &_setup->origin_offset_x;
	double* axisPtr = &_setup->axis_offset_x;

	if(!g_hmapFile)
	{
		return;
	}

	for(int ct=0; ct< MAX_AXES; ct++)
	{
		g_axes[ct].scale = MainPlanner->StepsPerAxis[ct];
		g_axes[ct].offset = *orgPtr++ + *axisPtr++;

		g_mappedStatus->velocities[ct] = GetDRO(806 + ct);
	}
	

	while(g_cmdData.Get(cmd))
	{
		HandleCommand(cmd,true);
	}

	static bool wasJog = false;
	bool isJog = false;

	for(int ct=0; ct< MAX_AXES; ct++)
	{
		AXISDATA& axis = g_axes[ct];
		AxisInfo& eng = Engine->Axis[ct];
		bool moving = false;
		if(!axis.jogging)
		{
			continue;
		}
		if(eng.Index != axis.index)
		{
			axis.index = eng.Index;
			moving = true;
		}
		if(axis.targetJog == 0)
		{
			if(!moving)
			{
				axis.jogging = false;
			}//else
			{
				eng.Dec = true;
				eng.Jogging = true;
//				isJog = true;
			}
		}else
		{
			isJog = true;
			bool dir = axis.targetJog < 0;
			if(dir == (eng.JoggDir == 0) && eng.CurVelocity != 0)
			{
				eng.Dec = true;
				eng.Jogging = true;
				continue;
			}
			eng.Acceleration = axis.acc;
			eng.JoggDir = dir;
			eng.MaxVelocity = abs(axis.targetJog);
			eng.Dec = false;
			eng.Jogging = true;
		}
	}
	if(isJog)
	{
		Code("DOJOG");
		wasJog = true;
	}else
	{
			Code("ENDJOG");
		if(wasJog)
		{
			wasJog = false;
		}
	}

	if(g_mappedStatus->moving && g_isMoving)
	{
		g_isMoving--;
		if(g_isMoving == 0)
		{
			g_mappedStatus->moving = false;
		}
	}
	g_mappedStatus->enabled = !Engine->EStop;

*/	
}


#endif

//---------------------------------------------------------------------
//
//	piNotify() - Plugin extension of Notify()
//
//		among other notices this is where we are notified when the
//		user clicks on our 'PlugIn Control' menu item.
//
//		XML file CAN be accessed on a menu item notify
//
//---------------------------------------------------------------------

#ifdef PI_NOTIFY
#ifdef _MANAGED
#pragma PI_MIX_NOTIFY
#endif
void piNotify(int id)
{
}
#endif

//---------------------------------------------------------------------
//
//	piDoDwell() - Plugin extension of DoDwell()
//
//---------------------------------------------------------------------

#ifdef PI_DODWELL
#ifdef _MANAGED
#pragma PI_MIX_DODWELL
#endif
void piDoDwell(double time)
{
}
#endif

//---------------------------------------------------------------------
//
//	piReset() - Plugin extension of Reset()
//
//---------------------------------------------------------------------

#ifdef PI_RESET
#ifdef _MANAGED
#pragma PI_MIX_RESET
#endif
void piReset()
{
}
#endif

//---------------------------------------------------------------------
//
//	piJogOn() - Plugin extension of JogOn()
//
//---------------------------------------------------------------------

#ifdef PI_JOGON
#ifdef _MANAGED
#pragma PI_MIX_JOGON
#endif
void piJogOn(short axis, short dir, double speed)
{
}
#endif

//---------------------------------------------------------------------
//
//	piJogOff() - Plugin extension of JogOff()
//
//---------------------------------------------------------------------

#ifdef PI_JOGOFF
#ifdef _MANAGED
#pragma PI_MIX_JOGOFF
#endif
void piJogOff(short axis)
{
}
#endif

//---------------------------------------------------------------------
//
//	piPurge() - Plugin extension of Purge()
//
//---------------------------------------------------------------------

#ifdef PI_PURGE
#ifdef _MANAGED
#pragma PI_MIX_PURGE
#endif
void piPurge(short flags)
{
}
#endif

//---------------------------------------------------------------------
//
//	piProbe() - Plugin extension of Probe()
//
//---------------------------------------------------------------------

#ifdef PI_PROBE
#ifdef _MANAGED
#pragma PI_MIX_PROBE
#endif
void piProbe()
{
}
#endif

//---------------------------------------------------------------------
//
//	piHome() - Plugin extension of Home()
//
//---------------------------------------------------------------------

#ifdef PI_HOME
#ifdef _MANAGED
#pragma PI_MIX_HOME
#endif
void piHome(short axis)
{
}
#endif
