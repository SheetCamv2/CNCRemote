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
#include <iostream>
#include <ctime>
#include <chrono>


#ifdef _WIN32
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <sstream>
#include "timer.h"

#define CATCHRPC 	catch (rpc::rpc_error &e) {\
	OnException(e);\
}catch(clmdep_msgpack::type_error & e) {\
	OnException(e);\
}catch(rpc::timeout & e) {\
	OnException(e);\
}


namespace CncRemote
{

Client::Client()
{
#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
	m_client = NULL;
	m_connected = false;
	m_serverVer = 0;
	m_errIndex = 0;
	m_msgIndex = 0;
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
	delete m_client;
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
    HANDLE hFind = FindFirstFileW((path + L"*.dll").c_str(), &dir);
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

void Client::SetBusy(const int state)
{
	if(state > m_statusCache)
	{
		m_statusCache = state; 
	}
	m_busyHeart = m_heartBeat+ 1;
}

COMERROR Client::Poll()
{
#ifdef USE_PLUGINS
    if(m_plugin) m_plugin->Poll();
#endif

	if (!m_connected) return errCONNECT;

	COMERROR ret = errOK;

	try
	{
		if (m_pollFuture.valid())
		{	
			int time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_pollTimer).count();
			if (m_pollFuture.wait_for(std::chrono::seconds(1)) == std::future_status::ready)
			{
				m_roundTrip = time;
				if (m_serverVer == 0)
				{
					float ver = m_pollFuture.get().as<float>();
					m_serverVer = ver;
					if (ver < CNCREMOTE_MIN_PROTOCOL_VERSION)
					{
						OnVersionFailed(ver);
						m_connected = false;
						m_state.machineStatus = mcNO_SERVER;
						m_serverVer = 0;
					}
				}
				else
				{
					m_state = m_pollFuture.get().as<State>();
					if (m_heartBeat >= m_busyHeart)
					{
						m_statusCache = 0;
					}
					m_heartBeat++;
				}
			}
			else
			{
				if (time > 1000000) //Time out if no response after one second and try again
				{
//					ret = errCONNECT;
					m_state.machineStatus = mcOFFLINE;
				}
				else
				{
					return errOK;
				}
			}
		}
	}CATCHRPC;

	try {
		m_pollTimer = std::chrono::high_resolution_clock::now();
		if (m_serverVer == 0)
		{
			m_pollFuture = m_client->async_call("Version");
		}
		else
		{
			m_pollFuture = m_client->async_call("GetState");
		}
	}
	catch (rpc::timeout& e)
	{
		int a = 1;
	}
	catch (clmdep_msgpack::type_error & e)
	{
		cout << e.what() << std::endl;
	}

    return ret;
}


void Client::OnException(rpc::rpc_error &e)
{
	std::cout << std::endl << e.what() << std::endl;
	std::cout << "in function '" << e.get_function_name() << "': "
		<< e.get_error().as<std::string>() << std::endl;

}

void Client::OnException(clmdep_msgpack::type_error & e)
{
	std::cout << "Type error: " << e.what() << std::endl;
}

void Client::OnException(rpc::timeout &e)
{
	std::cout << "Timeout error: " << e.what() << std::endl;
}

//void(rpc::client &, connection_state, connection_state)

void Client::OnConnectChange(rpc::client & client, rpc::connection_state was, rpc::connection_state now)
{
	if (was == rpc::connection_state::connected)
	{
		m_pollFuture = std::future<RPCLIB_MSGPACK::object_handle>(); //invalidate future because the server disconnected
	}
	if (now == rpc::connection_state::disconnected)
	{
		client.async_reconnect();
	}
	m_connected = now == rpc::connection_state::connected;
	if (!m_connected) m_state.machineStatus = mcNO_SERVER;
	m_serverVer = 0;
}




bool Client::Connect(const unsigned int index, const CncString& address, const uint32_t port)
{
	m_port = port;
	delete m_client;
	m_pollFuture = std::future<RPCLIB_MSGPACK::object_handle>(); //invalidate future
	m_address = to_utf8(address);
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
		m_address = "localhost";
	}
#endif

	m_client = new rpc::client(m_address, port, [this](rpc::client & client, rpc::connection_state was, rpc::connection_state now) {OnConnectChange(client, was, now); });
	m_client->set_timeout(500);
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
	delete m_client;
	m_client = NULL;
}

float Client::Ping(int waitMs)
{
	if (!m_connected) return -1;
	using namespace std::chrono;
	high_resolution_clock::time_point t = high_resolution_clock::now();
	try
	{
		auto future = m_client->async_call("Ping");
		future.wait_for(milliseconds(waitMs));
		if (future.valid())
		{
			return (float)(duration_cast<microseconds>(high_resolution_clock::now() - t).count()) / 1000;
		}
	}CATCHRPC;
	return -1;
}

bool Client::IsBusy(const int state)
{
	if (!m_connected) return true;
	if(m_statusCache > m_state.machineStatus)
	{
		return(m_statusCache > state);
	}
	return (m_state.machineStatus > state);
}

void Client::DrivesOn(const bool state)
{
	if (!m_connected) return;
	try
	{
		m_client->send("DrivesOn", state);
	}CATCHRPC;
}

void Client::JogVel(const Axes& velocities)
{
	if (!m_connected) return;
	try
	{
		m_client->send("JogVel", velocities);
	}CATCHRPC;
	SetBusy(mcJOGGING);
}

bool Client::Mdi(const string line)
{
	if (!m_connected) return false;
	bool ret = false;
	try
	{
		ret = m_client->call("Mdi", line).as<bool>();
	}CATCHRPC;
	SetBusy(mcMDI);
	return ret;
}


void Client::FeedOverride(const double percent)
{
	if (!m_connected) return;
	try
	{
		m_client->send("FeedOverride", percent);
	}CATCHRPC;
}

void Client::SpindleOverride(const double percent)
{
	if (!m_connected) return;
	try
	{
		m_client->send("SpindleOverride", percent);
	}CATCHRPC;
}

void Client::RapidOverride(const double percent)
{
	if (!m_connected) return;
	try
	{
		m_client->send("RapidOverride", percent);
	}CATCHRPC;
}


bool Client::LoadFile(string file)
{
	if (!m_connected) return false;
	if (!IsLocal()) //need to upload
	{
		FILE * fp = fopen(file.c_str(), "rb");
		if (!fp)
		{
			return false;
		}

#ifdef _WIN32
		int s = file.find_last_of("/\\");
#else
		int s = file.find_last_of("/");
#endif
		if (s == string::npos) s = 0;
		int e = file.find_last_of(".");
		if (e == string::npos || e < s) e = file.size();
		string name = file.substr(s + 1 , e - (s + 1));
		try 
		{
			file = m_client->call("SendInit", name).as<string>();
			if (file.empty()) return false;
			char buf[FILE_BLOCK_SIZE];
			string s;
			int blk = 0;
			int bytes = 0;
			do
			{
				bytes = fread(buf, 1, FILE_BLOCK_SIZE, fp);
				s.assign(buf, bytes);
				if (!m_client->call("SendData", s, blk++).as<bool>()) return false;
				Poll();
			} while (bytes == FILE_BLOCK_SIZE);
		}CATCHRPC;
	}


	bool ret = false;
	try
	{
		ret = m_client->call("LoadFile", file).as<bool>();
	}CATCHRPC;
	return ret;
}

void Client::CloseFile()
{
	if (!m_connected) return;
	try
	{
		m_client->send("CloseFile");
	}CATCHRPC;
}


void Client::CycleStart()
{
	if (!m_connected) return;
	try
	{
		m_client->send("CycleStart");
	}CATCHRPC;
	SetBusy(mcRUNNING);
}

void Client::CycleStop()
{
	if (!m_connected) return;
	try
	{
		m_client->send("CycleStop");
	}CATCHRPC;
}

void Client::FeedHold(const bool state)
{
	if (!m_connected) return;
	try
	{
		m_client->send("FeedHold", state);
	}CATCHRPC;
}


void Client::BlockDelete(const bool state)
{
	if (!m_connected) return;
	try
	{
		m_client->send("BlockDelete", state);
	}CATCHRPC;
}

void Client::SingleStep(const bool state)
{
	if (!m_connected) return;
	try
	{
		m_client->send("SingleStep", state);
	}CATCHRPC;
}

void Client::OptionalStop(const bool state)
{
	if (!m_connected) return;
	try
	{
		m_client->send("OptionalStop", state);
	}CATCHRPC;
}

void Client::Home(const unsigned int axis)
{
	if (!m_connected) return;
	if (axis >= MAX_AXES) return;
	BoolAxes axes;
	axes.Zero();
	axes.array[axis] = true;
	if (!m_connected) return;
	try
	{
		m_client->send("Home", axes);
	}CATCHRPC;
	SetBusy(mcRUNNING);
}

void Client::HomeAll()
{
	
	BoolAxes axes;
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		axes.array[ct] = true;
	}
	if (!m_connected) return;
	try
	{
		m_client->send("Home", axes);
	}CATCHRPC;
	SetBusy(mcRUNNING);
}

string Client::GetNextError()
{
	if (m_errIndex >= m_state.errorCount)
	{
		return string();
	}
	return m_client->call("GetError", m_errIndex++).as<string>();
}

string Client::GetNextMessage()
{
	if (m_msgIndex >= m_state.messageCount)
	{
		return string();
	}
	return m_client->call("GetMessage", m_msgIndex++).as<string>();
}



bool Client::IsLocal()
{
	//FIXME: Find a better way of detecting a local connection
	return (m_connected && (m_address == "localhost" || m_address == "127.0.0.1"));
}

} //namespace CncRemote
