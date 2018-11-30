#include "stdafx.h"
#include "MachServer.h"
#include "MachDevice.h"
#include <stdlib.h>
#include <process.h>    /* _beginthread, _endthread */
#include <math.h>
#include "tinyxml2.h"

/*
NOTE: Before you use the plugin you need to create these registry entries if they don't exist
Later Mach installers don't do this automatically.

[HKEY_CLASSES_ROOT\CLSID\{CA7992B2-2653-4342-8061-D7D385C07809}]
@="Mach4.Document"

[HKEY_CLASSES_ROOT\CLSID\{CA7992B2-2653-4342-8061-D7D385C07809}\InprocHandler32]
@="ole32.dll"

[HKEY_CLASSES_ROOT\CLSID\{CA7992B2-2653-4342-8061-D7D385C07809}\LocalServer32]
@="C:\\Mach3\\Mach3.exe"

[HKEY_CLASSES_ROOT\CLSID\{CA7992B2-2653-4342-8061-D7D385C07809}\ProgID]
@="Mach4.Document"

[HKEY_CLASSES_ROOT\Mach4.Document]
@="Mach4.Document"

[HKEY_CLASSES_ROOT\Mach4.Document\CLSID]
@="{CA7992B2-2653-4342-8061-D7D385C07809}"
*/


using namespace CncRemote;

MachServer::MachServer()
{
	running = true;
	m_Mach4App = NULL;
	m_Mach4Scripter = NULL;
	memset(m_jogAxes, 0, sizeof(m_jogAxes));

	if (!ProfileName || !LoadSettings())
	{
		LogError("Failed to load profile");
	}
	*m_filePath = 0;
}

MachServer::~MachServer()
{
	running = false;
}


void MachServer::StartLoop()
{
	HRESULT hr = CreateObject(OLESTR("Mach4.Document"), &m_Mach4App);
	if (FAILED(hr))
	{
		LogError("OLE registry keys missing. Install registry keys.");
		return;
	}
	VARIANT v;
	hr = Invoke(m_Mach4App,DISPATCH_PROPERTYGET,&v,NULL,NULL,OLESTR("GetScriptDispatch"),TEXT(""));
	if (FAILED(hr))
	{
		return;
	}
	m_Mach4Scripter = v.pdispVal;
}

void MachServer::Poll()
{
	for(int ct=0; ct< MAX_AXES; ct++)
	{
		AXISDATA& axis = m_jogAxes[ct];
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


	LockedState lockedState = GetState();
	State& state = lockedState.Data();

	if (Engine->CurrentUnits == 1)
	{
		state.gcodeUnits = 1/25.4;
	}
	else
	{
		state.gcodeUnits = 1;
	}
	state.feedHold = GetLED(111);
	state.feedOverride = GetDRO(821) / 100;
	state.optionalStop = GetLED(65);
	state.blockDelete = GetLED(66);
	if (Engine->EStop)
	{
		state.machineStatus = mcOFF;
	}else
	{
		if (Engine->TrajIndex != Engine->TrajHead)
		{
			state.machineStatus = mcRUNNING;
		}
		else
		{
			state.machineStatus = mcIDLE;
			for (int ct = 0; ct < MAX_AXES; ct++)
			{
				if (Engine->Axis[ct].Jogging)
				{
					state.machineStatus = mcJOGGING;
					break;
				}
			}
		}
	}
	state.currentLine = Engine->DisplayLine;
	state.singleStep = GetLED(82);
	state.spindleActual = GetDRO(39);
	state.spindleCmd = GetDRO(817);
	if (GetLED(164)) state.spindleState = spinFWD;
	else if(GetLED(165)) state.spindleState = spinREV;
	else state.spindleState = spinOFF;
	state.mist = GetLED(12);
	state.flood = GetLED(13);

	state.spindleOverride = GetDRO(74) / 100;
	state.rapidOverride = GetDRO(223) / 100;

	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		state.homed.array[ct] = Engine->Referenced[ct];
		state.position.array[ct] = GetDRO(800+ ct);
	}

	Server::Poll();
}


void MachServer::LoadThread(void * param) //poll axis positions quickly
{
	MachServer * svr = (MachServer *)param;

	if (svr->running == 1 && svr->m_filePath[0] != 0)
	{
		LockedState lockedState = svr->GetState();
		State& state = lockedState.Data();
		state.machineStatus = mcRUNNING;
		if (svr->m_Mach4App)
		{
			Invoke(svr->m_Mach4App, DISPATCH_METHOD, NULL, NULL, NULL, OLESTR("LoadGCodeFile"), TEXT("s"), svr->m_filePath);
		}
		svr->m_filePath[0] = 0;
		state.machineStatus = mcIDLE;
	}
}

void MachServer::UpdateState(State& state)
{
	if (Engine->CurrentUnits == 1)
	{
		for (int ct = 0; ct < MAX_AXES; ct++)
		{
			state.machinePos.array[ct] = (Engine->Axis[ct].Index / MainPlanner->StepsPerAxis[ct]) * 25.4;
		}
	}
	else
	{
		for (int ct = 0; ct < MAX_AXES; ct++)
		{
			state.machinePos.array[ct] = Engine->Axis[ct].Index / MainPlanner->StepsPerAxis[ct];
		}
	}
}

void MachServer::DrivesOn(const bool state)
{
	LockedState lockedState = GetState();
	bool cur = lockedState.Data().machineStatus > mcOFF;
	if(state != cur) DoButton(1021);
}

void MachServer::JogVel(const Axes velocities)
{
/*
	if (m_Mach4App)
	{
		for (int ct = 0; ct < MAX_AXES; ct++)
		{
			double v = velocities[ct];
			if (v == 0)
			{
				Invoke(m_Mach4App, DISPATCH_METHOD, NULL, NULL, NULL, OLESTR("JogOff"), TEXT("I"), ct);
			}
			else
			{
				Invoke(m_Mach4App, DISPATCH_METHOD, NULL, NULL, NULL, OLESTR("JogOn"), TEXT("II"), v < 0);
			}
		}
	}*/

	double linScale = 2048;
	double accScale = 2048 / (double)(Engine->PulseCount * 2);

	bool moving = false;

	for(int ct=0; ct<MAX_AXES; ct++)
	{
		AXISDATA& axis = m_jogAxes[ct];
		double cmd = velocities.array[ct] * MainPlanner->Velocities[ct];
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

		double stepsPerAxis = MainPlanner->StepsPerAxis[ct];

		axis.acc = (int)(MainPlanner->Acceleration[ct] * stepsPerAxis * accScale); 

		int jogAmount = (int)(cmd * stepsPerAxis * linScale);
		axis.targetJog = jogAmount;
		if(jogAmount != 0)
		{
			moving = true;
			axis.jogging = true;
		}
	}
}

void MachServer::JogStep(const Axes distance, const double speed)
{
	//TODO: Try to figure out a good way to step jog!
/*	string s;
	LockedState ls = GetState();
	double units = ls.Data().gcodeUnits;
	const char axes[] = "XYZABC";
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		double d = distance[ct];
		if (d == 0) continue;
		char buf[100];
		sprintf(buf,"%c%f", axes[ct], d * )
	}*/
}

bool MachServer::Mdi(const string line)
{
	LockedState lockedState = GetState();
	State& state = lockedState.Data();
	if (state.machineStatus <= mcOFF || state.machineStatus >= mcRUNNING) return false;
	Code(line.c_str());
	return true;
}

void MachServer::SpindleOverride(const double percent)
{
	Sync();
	SetDRO(74, percent * 100);
}

void MachServer::FeedOverride(const double percent)
{
	Sync();
	SetDRO(821, percent * 100);
}


void MachServer::RapidOverride(const double percent)
{
	Sync();
	SetDRO(223, percent * 100);
}

bool MachServer::LoadFile(const string file)
{
	LockedState lockedState = GetState();
	State& state = lockedState.Data();
	if(!m_Mach4Scripter || state.machineStatus >= mcRUNNING) return false;
	if(0 == MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED, file.c_str(), -1, m_filePath, MAX_PATH))
	{
		m_filePath[0] = 0;
		return false;
	}
	state.machineStatus = mcRUNNING;
	_beginthread(LoadThread, 0, this); //load is slow so run it asynchronously
	return true;
}


bool MachServer::CloseFile()
{
	LockedState lockedState = GetState();
	State& state = lockedState.Data();
	if(state.machineStatus >= mcRUNNING) return false;
	Sync();
	DoButton(169);
	return true;
}

void MachServer::CycleStart()
{
	Sync();
	DoButton(1000);
}


void MachServer::CycleStop()
{
	Sync();
	DoButton(1003);
	DoButton(1002);
}

void MachServer::FeedHold(const bool state)
{
	Sync();
	bool cur = GetLED(111);
	if(state != cur) DoButton(1001);
}

void MachServer::BlockDelete(const bool state)
{
	Sync();
	bool cur = GetLED(66);
	if(state != cur) DoButton(176);
}

void MachServer::SingleStep(const bool state)
{
	Sync();
	bool cur = GetLED(82);
	if(state != cur) DoButton(1004);
	SetLED(82, state);
}

void MachServer::OptionalStop(const bool state)
{
	Sync();
	SetLED(65, state);
	bool cur = GetLED(65);
	if(state != cur) DoButton(177);
}

void MachServer::Home(const BoolAxes axes)
{
	Sync();
	bool all = true;
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		if (!axes.array[ct])
		{
			all = false;
			break;
		}
	}
	if (all)
	{
		DoButton(105); //home all
		return;
	}

//		state.gcodeUnits
	LockedState lockedState = GetState();
	State& state = lockedState.Data();

	char buf[128] = "G28.1";
	double feed = 0;
	const char axChars[] = "XYZABC";

	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		if (axes.array[ct])
		{
			sprintf(buf, "%s %c%f", buf, axChars[ct], 50 * state.gcodeUnits);
			double f = m_jogAxes[ct].homeSpeed * MainPlanner->Velocities[ct] * 60;
			feed += (f * f);
		}
	}
	if (feed == 0) return;
	feed = sqrt(feed);
	sprintf(buf, "%s F%f", buf, feed * state.gcodeUnits);
	Code(buf);
}

Axes MachServer::GetOffset(const unsigned int index)
{
	Axes ret;
	double * ptr = NULL;
	switch (index)
	{
	case 1:
		ptr =  &_setup->axis_offset_x;
		break;

	case 2:
		ptr =  &_setup->origin_offset_x;
		break;

	default:
		break;
	}

	if (ptr)
	{
		for (int ct = 0; ct < MAX_AXES; ct++)
		{
			ret.array[ct] = *ptr++;
		}
	}else
	{
		memset(&ret, 0, sizeof(ret));
	}
	return ret;
}

bool MachServer::LoadSettings()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(ProfileName);
	auto prof = doc.FirstChildElement("profile");
	if (!prof) return false;
	auto * prefs = prof->FirstChildElement("Preferences");
	if (!prefs) return false;
	char buf[16];
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		sprintf(buf, "RefSpeed%d", ct);
		auto e = prefs->FirstChildElement(buf);
		if (!e) return false;
		const char * t = e->GetText();
		if (e->QueryDoubleText(&m_jogAxes[ct].homeSpeed) != tinyxml2::XML_SUCCESS) return false;
		m_jogAxes[ct].homeSpeed /= 100;
	}
	return true;
}
