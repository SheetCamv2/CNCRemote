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


#ifdef _WIN32
#include <WinSock2.h>
#else
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>

#endif


#include "cncclient.h"
#include <iostream>
#include <ctime>



#include <sstream>
#include "timer.h"

namespace CncRemote
{
#ifdef HANDLE_CNCREMOTE_EXCEPTIONS
#define TRY_EXCEPTION() try{
#define CATCH_EXCEPTION()	}catch (std::exception& exc) {OnException(exc);}
#define CATCH_EXCEPTION2()	catch (std::exception& exc) {OnException(exc);}
#else
#define TRY_EXCEPTION()
#define CATCH_EXCEPTION()
#define CATCH_EXCEPTION2()
#endif


RemoteCall::RemoteCall()
{
	using namespace std::placeholders;
	m_response = NULL;
	m_hasResponse = false;
	m_busy = false;
	m_onError = std::bind(&RemoteCall::OnError, this, _1, _2, _3);
	m_onResponse = std::bind(&RemoteCall::OnResponse, this, _1, _2);
}

linear::Response& RemoteCall::Call(linear::Socket& socket, unsigned timeout, const std::string& function, const linear::type::any& param)
{
	CallAsync(socket, timeout, function, param);
	if (!Wait(timeout))
	{
		throw(TimeoutError());
	}
	return GetResponse();
}


void RemoteCall::CallAsync(linear::Socket& socket, unsigned timeout, const std::string& function, const linear::type::any& param)
{
	if (m_busy)
	{
		throw(BusyError());
	}
	m_busy = true;
	m_hasResponse = false;
	m_error.clear();
	linear::Request request(function, param);
	linear::Error err = request.Send(socket, timeout, m_onResponse, m_onError);
	if (err != linear::LNR_OK)
	{
		throw(TransferError(err.Message()));
	}
}

linear::Response& RemoteCall::GetResponse()
{
	if (!m_hasResponse || !m_response)
	{
		throw(TimeoutError());
	}

	if (!m_response->error.is_nil())
	{
		throw(RemoteError(m_response->error.stringify()));
	}

	if (!m_error.empty())
	{
		throw(SendError(m_error));
	}
	return *m_response;
}


bool RemoteCall::Wait(unsigned ms)
{
	while (m_busy && ms > 0)
	{
		SleepMs(1);
		ms--;
	}
	return (!m_busy);
}

void RemoteCall::OnResponse(const linear::Socket& socket, const linear::Response& response)
{
	if (!m_response)
	{
		m_response = new linear::Response(response);
	}
	else
	{
		*m_response = response;
	}
	m_hasResponse = true;
	m_busy = false;
}

void RemoteCall::OnError(const linear::Socket& socket, const linear::Request& request, const linear::Error& error)
{
	m_error = error.Message();
	m_busy = false;
}


class Client::Handler : public linear::Handler
{
public:
	Handler(Client& client)
	{
		m_client = &client;
		m_retries = 1;
	}

	~Handler() {}

	static void Reconnect(void* args)
	{
		linear::Socket* socket = reinterpret_cast<linear::Socket*>(args);
		socket->Connect();
		delete socket;
	}

	void OnConnect(const linear::Socket& socket)
	{
		m_retries = 0;
//		const linear::Addrinfo& info = socket.GetPeerInfo();
	}

	void OnDisconnect(const linear::Socket& socket, const linear::Error&) {
		if (m_retries == 0)
		{
			m_client->OnDisConnect2();
		}
		int retryTime = 1000;
		if (m_retries < 10)
		{
			retryTime = 100;
			m_retries++;
		}
		m_retryTimer.Start(Handler::Reconnect, retryTime, new linear::Socket(socket));
	}

	void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
//		const linear::Addrinfo& info = socket.GetPeerInfo();
		switch(msg.type) {
		case linear::REQUEST:
		{
			linear::Request request = msg.as<linear::Request>();
			linear::Response response(request.msgid, linear::type::nil(), std::string("This client does not handle request"));
			response.Send(socket);
		}
		break;
		case linear::RESPONSE: //Unused
		{
//			linear::Response response = msg.as<linear::Response>();
		}
		break;
		case linear::NOTIFY: //Gneerally only for execptions
		{
			linear::Notify notify = msg.as<linear::Notify>();
			try {
				ExceptionData exc = notify.params.as<ExceptionData>();
				m_client->OnRemoteException(exc);
			} catch(const std::bad_cast&) {
				std::cout << "invalid type cast" << std::endl;
			}
		}
		break;
		default:
		{
			std::cout << "BUG: plz inform to linear-developers" << std::endl;
		}
		break;
		}
	}
	void OnError(const linear::Socket&, const linear::Message& msg, const linear::Error& err) {
		switch(msg.type) {
		case linear::REQUEST:
		{
			linear::Request request = msg.as<linear::Request>();
			std::cout << "Error to Send Request: msgid = " << request.msgid
				<< ", method = \"" << request.method << "\""
				<< ", params = " << request.params.stringify()
				<< ", err = " << err.Message() << std::endl;
		}
		break;
		case linear::RESPONSE:
		{
			linear::Response response = msg.as<linear::Response>();
			std::cout << "Error to Send Response: msgid = " << response.msgid
				<< ", result = " << response.result.stringify()
				<< ", error = " << response.error.stringify()
				<< ", err = " << err.Message() << std::endl;
			std::cout << "origin request: msgid = " << response.request.msgid
				<< ", method = \"" << response.request.method << "\""
				<< ", params = " << response.request.params.stringify() << std::endl;
		}
		break;
		case linear::NOTIFY:
		{
			linear::Notify notify = msg.as<linear::Notify>();
			std::cout << "Error to Send Notify: "
				<< "method = \"" << notify.method << "\""
				<< ", params = " << notify.params.stringify()
				<< ", err = " << err.Message() << std::endl;
		}
		break;
		default:
		{
			std::cout << "BUG: plz inform to linear-developers" << std::endl;
		}
		break;
		}
	}

private:
	int m_retries;
	linear::Timer m_retryTimer;
	Client * m_client;
};



Client::Client()
{

#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
	m_serverVer = 0;
	m_errIndex = 0;
	m_msgIndex = 0;

	m_handler = linear::shared_ptr<Handler>(new Handler(*this));
	m_client = linear::TCPClient(m_handler);

	m_timeout = 500;
}

Client::~Client()
{
#ifdef USE_PLUGINS
    Disconnect();
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

bool Client::LoadPlugins(const CncString& path, CNCLOGFUNC logFunc)
{
    if(m_plugins.size() > 0) return true; //Only enumerate plugins once
    if(!logFunc) return false;

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
logFunc(path.c_str());
    DIR* dir = opendir((path).c_str());
    if (dir == NULL)
    {
        return false;
    }
    dirent *file = readdir(dir);
    if(file == NULL)
    {
        return false;
    }
    do
    {
        if(file->d_type != DT_REG) continue;
        string dllName = path + "/" + file->d_name;
        LIBHANDLE handle = dlopen(dllName.c_str(), RTLD_LAZY);
logFunc(dllName.c_str());
        if(!handle)
        {
            logFunc(dlerror());
            continue;
        }
        Plugin plg;
        plg.handle = handle;
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
                plg.ControlExists((path + "/").c_str(), logFunc) == false)
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

	if (!IsConnected()) return errCONNECT;

	COMERROR ret = errOK;



	if (m_serverVer <= 0)
	{
		if (m_serverVer < 0) return errWRONGVER;
		if (m_StatusCall.HasResponse())
		{
			try
			{
				float ver = m_StatusCall.GetResponse().result.as<float>();
				if (ver < CNCREMOTE_MIN_PROTOCOL_VERSION)
				{
					OnIncorrectVersion(ver);
					m_serverVer = -1;
				}
				else
				{
					OnConnect();
					m_serverVer = ver;
				}
				m_StatusCall.ClearResponse();
			}
			catch (std::exception& exc) //ignore errors
			{
			}
		}
		if (!m_StatusCall.IsBusy() && m_serverVer == 0)
		{
			try
			{
				m_StatusCall.CallAsync(m_socket, m_timeout, "Version");
			}
			catch (std::exception& exc)
			{
			}
		}
		return ret;
	}





	if (m_StatusCall.HasResponse())
	{
		m_roundTrip = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_pollTimer).count();
		try
		{
			m_state = m_StatusCall.GetResponse().result.as<State>();
			if (m_heartBeat >= m_busyHeart)
			{
				m_statusCache = 0;
			}
			m_heartBeat++;
		}
		catch (std::exception& exc) //ignore errors
		{
		}
	}
	if (!m_StatusCall.IsBusy())
	{
		try
		{
			m_StatusCall.CallAsync(m_socket, m_timeout, "GetState");
		}
		catch (std::exception& exc)
		{
		}
		m_pollTimer = std::chrono::high_resolution_clock::now();
	}

    return ret;
}


bool Client::Connect(const unsigned int index, const CncString& address, const uint32_t port)
{
	std::string addr = to_utf8(address);
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
		addr = "127.0.0.1";
	}
#endif
	m_socket = m_client.CreateSocket(addr, port);
	m_socket.Connect();
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
}

float Client::Ping(int waitMs)
{
	using namespace std::chrono;
	RemoteCall remote;
	high_resolution_clock::time_point t = high_resolution_clock::now();
	TRY_EXCEPTION();
	remote.Call(m_socket, m_timeout, "Version", 0);
	if(remote.HasResponse())
	{
		return (float)(duration_cast<microseconds>(high_resolution_clock::now() - t).count()) / 1000;
	}
	CATCH_EXCEPTION();
	return -1;
}

bool Client::IsBusy(const int state)
{
	if (!IsConnected()) return true;
	if(m_statusCache > m_state.machineState)
	{
		return(m_statusCache > state);
	}
	return (m_state.machineState > state);
}

void Client::DrivesOn(const bool state)
{
	TRY_EXCEPTION();
	linear::Notify notify("DrivesOn", state);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::JogVel(const Axes& velocities)
{
	TRY_EXCEPTION();
	linear::Notify notify("JogVel", velocities);
	if (notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcMOVING);
	}
	CATCH_EXCEPTION();
}

bool Client::Mdi(const string line)
{
	TRY_EXCEPTION();
	RemoteCall c;
	c.Call(m_socket, m_timeout, "Mdi", line);
	SetBusy(mcMDI);
	return c.GetResponse().result.as<bool>();
	CATCH_EXCEPTION();
	return false;
}


void Client::FeedOverride(const double percent)
{
	TRY_EXCEPTION();
	linear::Notify notify("FeedOverride", percent);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::SpindleOverride(const double percent)
{
	TRY_EXCEPTION();
	linear::Notify notify("SpindleOverride", percent);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::RapidOverride(const double percent)
{
	TRY_EXCEPTION();
	linear::Notify notify("RapidOverride", percent);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}


bool Client::LoadFile(string file)
{
	TRY_EXCEPTION();
	if (!IsConnected()) return false;
	RemoteCall remote;
	if (!IsLocal()) //need to upload
	{
		FILE * fp = fopen(file.c_str(), "rb");
		if (!fp)
		{
			return false;
		}

#ifdef _WIN32
		size_t s = file.find_last_of("/\\");
#else
		size_t s = file.find_last_of("/");
#endif
		if (s == string::npos) s = 0;
		size_t e = file.find_last_of(".");
		if (e == string::npos || e < s) e = file.size();
		string name = file.substr(s + 1 , e - (s + 1));
		file = remote.Call(m_socket, m_timeout, "SendInit", name).result.as<string>();
		if (file.empty()) return false;
		char buf[FILE_BLOCK_SIZE];
		size_t bytes = 0;
		int block = 0;
		string data;
		do
		{
			bytes = fread(buf, 1, FILE_BLOCK_SIZE, fp);
			data.assign(buf, bytes);
			if (!remote.Call(m_socket, m_timeout, "SendData", data, block).result.as<bool>()) return false;
			block++;
			Poll();
		} while (bytes == FILE_BLOCK_SIZE);
	}
	return (remote.Call(m_socket, m_timeout, "LoadFile", file).result.as<bool>());
	CATCH_EXCEPTION();
	return false;
}

bool Client::CloseFile()
{
	TRY_EXCEPTION();
	RemoteCall remote;
	return remote.Call(m_socket, m_timeout, "CloseFile", 0).result.as<bool>();
	CATCH_EXCEPTION();
	return false;
}


void Client::CycleStart()
{
	TRY_EXCEPTION();
	linear::Notify notify("CycleStart", true);
	if (notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
	CATCH_EXCEPTION();
}

void Client::CycleStop()
{
	TRY_EXCEPTION();
	linear::Notify notify("CycleStart", false);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::FeedHold(const bool state)
{
	TRY_EXCEPTION();
	linear::Notify notify("FeedHold", state);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}


void Client::BlockDelete(const bool state)
{
	TRY_EXCEPTION();
	linear::Notify notify("BlockDelete", state);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::SingleStep(const bool state)
{
	TRY_EXCEPTION();
	linear::Notify notify("SingleStep", state);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::OptionalStop(const bool state)
{
	TRY_EXCEPTION();
	linear::Notify notify("OptionalStop", state);
	notify.Send(m_socket);
	CATCH_EXCEPTION();
}

void Client::Home(const unsigned int axis)
{
	if (axis >= MAX_AXES) return;
	BoolAxes axes;
	axes.Zero();
	axes.array[axis] = true;
	TRY_EXCEPTION();
	linear::Notify notify("Home", axes);
	if(notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
	CATCH_EXCEPTION();
}

void Client::HomeAll()
{

	BoolAxes axes;
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		axes.array[ct] = true;
	}
	TRY_EXCEPTION();
	linear::Notify notify("Home", axes);
	if(notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
	CATCH_EXCEPTION();
}


vector<int> Client::GetGCodes()
{
	TRY_EXCEPTION();
	RemoteCall c;
	c.Call(m_socket, m_timeout, "GetGCodes");
	return c.GetResponse().result.as<vector<int>>();
	CATCH_EXCEPTION();
	vector<int> ret;
	return ret;
}

void Client::GetGCodes(RemoteCall& call)
{
	call.ClearResponse();
	call.Call(m_socket, m_timeout, "GetGCodes");
}


vector<int> Client::GetMCodes()
{
	TRY_EXCEPTION();
	RemoteCall c;
	c.Call(m_socket, m_timeout, "GetMCodes");
	return c.GetResponse().result.as<vector<int>>();
	CATCH_EXCEPTION();
	vector<int> ret;
	return ret;
}

void Client::GetMCodes(RemoteCall& call)
{
	call.ClearResponse();
	call.Call(m_socket, m_timeout, "GetGCodes");
}


string Client::GetNextError()
{
	string ret;
	if (m_errIndex >= m_state.errorCount)
	{
		return ret;
	}
	TRY_EXCEPTION();
	RemoteCall remote;
	ret = remote.Call(m_socket, m_timeout, "GetError", m_errIndex).result.as<string>();
	m_errIndex++;
	CATCH_EXCEPTION();
	return ret;


}

string Client::GetNextMessage()
{
	string ret;
	if (m_msgIndex >= m_state.messageCount)
	{
		return ret;
	}
	TRY_EXCEPTION();
	RemoteCall remote;
	ret = remote.Call(m_socket, m_timeout, "GetMessage", m_msgIndex).result.as<string>();
	m_msgIndex++;
	CATCH_EXCEPTION();
	return ret;
}



bool Client::IsLocal()
{
	const linear::Addrinfo& peer = m_socket.GetPeerInfo();
	const linear::Addrinfo& self = m_socket.GetSelfInfo();
	return peer.addr == self.addr;
}


void Client::OnDisConnect2()
{
	m_serverVer = 0;
	m_state.machineState = mcNO_SERVER;
	OnDisConnect();
}

} //namespace CncRemote
