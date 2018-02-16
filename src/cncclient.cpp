/****************************************************************
CNCRemote client
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




#include "cncclient.h"
#include <time.h>

#ifdef _WIN32
#include "Shlwapi.h"
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <sstream>
#include "timer.h"

namespace CncRemote
{

Client::Client()
{
#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
    SetSocket(new CActiveSocket());
}

Client::~Client()
{
#ifdef USE_PLUGINS

    for(size_t ct=0; ct < m_plugins.size(); ct++)
    {
        Plugin &plg = m_plugins[ct];
        plg.Quit();
#ifdef _WIN32
        FreeLibrary(plg.handle);
#else
        dlclose(plg.handle);
#endif
    }
#endif
}


#ifdef USE_PLUGINS

bool Client::LoadPlugins(const CncString& path)
{
    if(m_plugins.size() > 0) return true; //Only enumerate plugins once

#ifdef _WIN32
    /*
    	WCHAR buf[MAX_PATH];
        DWORD length = GetModuleFileNameW( NULL, buf, MAX_PATH );
    //#if (NTDDI_VERSION >= NTDDI_WIN8)
    //    PathCchRemoveFileSpecW(buf, MAX_PATH);
    //#else
        PathRemoveFileSpecW(buf);
    //#endif
        wstring path(buf);
        path += L"\\plugins\\";
    */


    WIN32_FIND_DATAW dir;
    HANDLE hFind = FindFirstFileW((path + L"\\*.dll").c_str(), &dir);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    do
    {
        Plugin plg;
        wstring dllName = path + dir.cFileName;
        plg.handle = LoadLibraryW(dllName.c_str());
        if(!plg.handle) continue;
        plg.Start = (CNCSTARTFUNC) GetProcAddress(plg.handle,"Start");
        plg.Stop = (CNCSTOPFUNC) GetProcAddress(plg.handle,"Stop");
        plg.GetName = (CNCGETNAMEFUNC) GetProcAddress(plg.handle,"GetName");
        plg.Quit = (CNCQUITFUNC) GetProcAddress(plg.handle,"Quit");
        plg.Poll = (CNCPOLLFUNC) GetProcAddress(plg.handle,"Poll");
        plg.ControlExists = (CNCCONTROLEXISTSFUNC) GetProcAddress(plg.handle,"ControlExists");
        if(!plg.Start ||
                !plg.Stop||
                !plg.GetName||
                !plg.Quit ||
                !plg.Poll ||
                !plg.ControlExists ||
                plg.ControlExists(path.c_str()) == false)
        {
            FreeLibrary(plg.handle);
        }
        else
        {
            m_plugins.push_back(plg);
        }
    }
    while(FindNextFileW(hFind, &dir) != NULL);
    FindClose(hFind);
#else
    DIR* dir = opendir((path).c_str());
    if (dir == NULL)
    {
        return false;
    }
    dirent * file = readdir(dir);
    if(file == NULL)
    {
        return false;
    }
    do
    {
        Plugin plg;
        if(strcmp(file->d_name, ".") == 0 ||
           strcmp(file->d_name, "..") == 0)
        {
            continue;
        }
        string dllName = path + "/" + file->d_name;
        plg.handle = dlopen(dllName.c_str(), RTLD_NOW);
        if(!plg.handle)
        {
            char * msg = dlerror();
            printf("%s\n", msg);
            continue;
        }
        plg.Start = (CNCSTARTFUNC) dlsym(plg.handle,"Start");
        plg.Stop = (CNCSTOPFUNC) dlsym(plg.handle,"Stop");
        plg.GetName = (CNCGETNAMEFUNC) dlsym(plg.handle,"GetName");
        plg.Quit = (CNCQUITFUNC) dlsym(plg.handle,"Quit");
        plg.Poll = (CNCPOLLFUNC) dlsym(plg.handle,"Poll");
        plg.ControlExists = (CNCCONTROLEXISTSFUNC) dlsym(plg.handle,"ControlExists");
        if(!plg.Start ||
                !plg.Stop||
                !plg.GetName||
                !plg.Quit ||
                !plg.Poll ||
                !plg.ControlExists ||
                plg.ControlExists((path + "/").c_str()) == false)
        {
            dlclose(plg.handle);
        }
        else
        {
            m_plugins.push_back(plg);
        }
    }
    while((file = readdir(dir)) != NULL);
    closedir(dir);
#endif
    return true;
}
#endif

void Client::HandlePacket(const Packet & pkt)
{
    m_serverHeart = pkt.hdr.heartBeat;
	if(m_busy &&
		(m_busyHeart - m_serverHeart) < 0 )//We are in sync with machine state
	{
		m_busy = false;
	}

	switch(pkt.hdr.cmd)
    {
    case cmdNULL:
        break;

    case cmdSTATE:
    {
        StateBuf buf;
        buf.ParseFromString(pkt.data);
        m_state.MergeFrom(buf);
    }
    break;
    }
}


bool Client::Poll()
{
#ifdef USE_PLUGINS
    if(m_plugin) m_plugin->Poll();
#endif

    COMERROR ret = Comms::Poll();
    /*switch(ret)
    {
    case errNODATA:
//        m_isConnected = (m_timeout > time(NULL));
        break;

    case errOK:
//        m_timeout = time(NULL) + CONN_TIMEOUT;
        break;

    default:
    }*/

//    if(m_isConnected)
    {
        SendCommand(cmdSTATE);
    }
    return ret == errOK;
}


bool Client::Connect(const unsigned int index, const CncString& address, const uint32_t port)
{
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
    if(index > 0)
    {
        if(index > m_plugins.size()) return false;
        m_plugin = &m_plugins[index - 1];
        m_plugin->Start();
        Comms::Connect(_T("localhost"),port);
    }else
#endif

    Comms::Connect(address, port);
    return true;
}

void Client::Disconnect()
{
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
#endif
    Close();
}

bool Client::Ping(int waitMs)
{
    m_pingResp = false;
    SendCommand(cmdPING);
    while (!m_pingResp && waitMs >= 0)
    {
        SleepMs(10);
        if(!Poll()) return false;
        waitMs -= 10;
    }
    return m_pingResp;
}

bool Client::IsBusy()
{
	if(m_busy) return true;
	return m_state.busy();
}

void Client::DrivesOn(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdDRIVES_ON, cmd);
	SetBusy();
}

void Client::JogVel(const Axes& velocities)
{
    CmdBuf cmd;
    *cmd.mutable_axes() = velocities;
    SendCommand(cmdJOG_VEL, cmd);
	SetBusy();
}

void Client::Mdi(const string line)
{
    CmdBuf cmd;
    cmd.set_string(line);
    SendCommand(cmdMDI, cmd);
	SetBusy();
}


void Client::FeedOverride(const double percent)
{
    CmdBuf cmd;
    cmd.set_rate(percent);
    SendCommand(cmdFRO, cmd);
}

void Client::SpindleOverride(const double percent)
{
    CmdBuf cmd;
    cmd.set_rate(percent);
    SendCommand(cmdFRO, cmd);
}

void Client::RapidOverride(const double percent)
{
    CmdBuf cmd;
    cmd.set_rate(percent);
    SendCommand(cmdFRO, cmd);
}


void Client::LoadFile(const string file)
{
    CmdBuf cmd;
    cmd.set_string(file);
    SendCommand(cmdFILE, cmd);
}

void Client::CloseFile()
{
    CmdBuf cmd;
    SendCommand(cmdCLOSE_FILE, cmd);
}


void Client::CycleStart()
{
    CmdBuf cmd;
    SendCommand(cmdSTART, cmd);
	SetBusy();
}

void Client::Stop()
{
    CmdBuf cmd;
    SendCommand(cmdSTOP, cmd);
}

void Client::FeedHold(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdFEED_HOLD, cmd);
}


void Client::BlockDelete(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdBLOCK_DEL, cmd);
}

void Client::SingleStep(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdSINGLE_STEP, cmd);
	SetBusy();
}

void Client::OptionalStop(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdOPT_STOP, cmd);
}

void Client::Home(const int axis)
{
	CmdBuf buf;
	BoolAxes& axes = *buf.mutable_bool_axes();
	switch(axis)
	{
	case 0:
		axes.set_x(true);
		break;

	case 1:
		axes.set_y(true);
		break;

	case 2:
		axes.set_z(true);
		break;

	case 3:		
		axes.set_a(true);
		break;

	case 4:		
		axes.set_b(true);
		break;

	case 5:		
		axes.set_c(true);
		break;

	default:
		return;
	}

	SendCommand(cmdHOME, buf);	
	SetBusy();
}

void Client::HomeAll()
{
	CmdBuf buf;
	BoolAxes& axes = *buf.mutable_bool_axes();
	axes.set_x(true);
	axes.set_y(true);
	axes.set_z(true);
	axes.set_a(true);
	axes.set_b(true);
	axes.set_c(true);
	SendCommand(cmdHOME, buf);	
	SetBusy();
}


bool Client::SendCommand(const uint16_t cmd)
{
    CmdBuf buf;
    Packet packet;
	packet.hdr.heartBeat = ++m_heartBeat;
    packet.hdr.cmd = cmd;
    return SendPacket(packet);
}

bool Client::SendCommand(const uint16_t cmd, const bool state)
{
    CmdBuf buf;
    buf.set_state(state);
    return SendCommand(cmd, buf);
}

bool Client::SendCommand(const uint16_t cmd, const double value)
{
    CmdBuf buf;
    buf.set_rate(value);
    return SendCommand(cmd, buf);
}

bool Client::SendCommand(const uint16_t cmd, const string value)
{
    CmdBuf buf;
    buf.set_string(value);
    return SendCommand(cmd, buf);
}


bool Client::SendCommand(const uint16_t command, CmdBuf& data)
{
    Packet packet;
	packet.hdr.heartBeat = ++m_heartBeat;
    data.SerializeToString(&packet.data);
    packet.hdr.cmd = command;
    return SendPacket(packet);
}


} //namespace CncRemote
