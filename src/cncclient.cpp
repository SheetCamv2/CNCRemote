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
#endif


#include "cncclient.h"
#include <iostream>
#include <ctime>
/*

#ifdef _WIN32
#else
#include <dlfcn.h>
#include <dirent.h>
#endif
*/


#include <sstream>
#include "timer.h"

namespace CncRemote
{

Client::ResponseTrack::ResponseTrack(Client* client, const int32_t msgid, linear::Response& response)
{
	m_data.client = client;
	m_data.ok = false;
	m_data.response = &response;
	m_data.msgid = msgid;
	m_data.mutex.lock();
	{
		linear::lock_guard<linear::mutex> guard(m_mutex);
		m_responses.push_back(&m_data);
	}
}

Client::ResponseTrack::~ResponseTrack()
{
	linear::lock_guard<linear::mutex> guard(m_mutex);
	for (std::vector<ResponseData *>::iterator it = m_responses.begin(); it != m_responses.end(); it++)
	{
		if ((*it) == &m_data)
		{
			m_responses.erase(it);
			return;
		}
	}
}

bool Client::ResponseTrack::Wait()
{
	m_data.mutex.lock(); //Wait for thread to unlock
	linear::lock_guard<linear::mutex> guard(m_mutex);
	for (std::vector<ResponseData *>::iterator it = m_responses.begin(); it != m_responses.end(); it++)
	{
		if ((*it) == &m_data)
		{
			m_responses.erase(it);
			return m_data.ok;
		}
	}
	return false;
}

Client::ResponseData* Client::ResponseTrack::Find(const int32_t msgid)
{
	linear::lock_guard<linear::mutex> guard(m_mutex);
	for (std::vector<ResponseData *>::iterator it = m_responses.begin(); it != m_responses.end(); it++)
	{
		if ((*it)->msgid == msgid)
		{
			return (*it);
		}
	}
	return NULL;
}


void Client::ResponseTrack::OnResponse(const linear::Socket& socket, const linear::Response& response)
{
	ResponseData* dat = ResponseTrack::Find(response.msgid);
	if (!dat) return; //Don't recognise this response
	*(dat->response) = response;
	dat->ok = true;
	dat->mutex.unlock();
}

void Client::ResponseTrack::OnError(const linear::Socket& socket, const linear::Request& request, const linear::Error& error)
{
	ResponseData* dat = ResponseTrack::Find(request.msgid);
	if (!dat) return; //Don't recognise this response
	dat->ok = false;
	dat->mutex.unlock();
}

std::vector<Client::ResponseData *> Client::ResponseTrack::m_responses;
linear::mutex Client::ResponseTrack::m_mutex;



class Client::Handler : public linear::Handler
{
public:
	Handler()
	{
		m_retries = 0;
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
		const linear::Addrinfo& info = socket.GetPeerInfo();
		std::cout << "OnConnect: " << info.addr << ":" << info.port << std::endl;
	}

	void OnDisconnect(const linear::Socket& socket, const linear::Error&) {
		int retryTime = 1000;
		if (m_retries < 10)
		{
			retryTime = 100;
			m_retries++;
		}
		m_retryTimer.Start(Handler::Reconnect, retryTime, new linear::Socket(socket));
	}
	void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
		const linear::Addrinfo& info = socket.GetPeerInfo();
		switch(msg.type) {
		case linear::REQUEST:
		{
			linear::Request request = msg.as<linear::Request>();
			std::cout << "recv Request: msgid = " << request.msgid
				<< ", method = \"" << request.method << "\""
				<< ", params = " << request.params.stringify()
				<< " from " << info.addr << ":" << info.port << std::endl;
			linear::Response response(request.msgid, linear::type::nil(), std::string("This client does not handle request"));
			response.Send(socket);
		}
		break;
		case linear::RESPONSE:
		{
			linear::Response response = msg.as<linear::Response>();
			std::cout << "recv Response(Handler): msgid = " << response.msgid
				<< ", result = " << response.result.stringify()
				<< ", error = " << response.error.stringify()
				<< " from " << info.addr << ":" << info.port << std::endl;
			std::cout << "origin request: msgid = " << response.request.msgid
				<< ", method = \"" << response.request.method << "\""
				<< ", params = " << response.request.params.stringify() << std::endl;
		}
		break;
		case linear::NOTIFY:
		{
			linear::Notify notify = msg.as<linear::Notify>();
			std::cout << "recv Notify: "
				<< "method = \"" << notify.method << "\""
				<< ", params = " << notify.params.stringify()
				<< " from " << info.addr << ":" << info.port << std::endl;
			try {
/*				Derived data = notify.params.as<Derived>();
				std::cout << "parameters detail" << std::endl;
				std::cout << "Base::"
					<< "int: " << data.int_val
					<< ", double: " << data.double_val
					<< ", string: " << data.string_val
					<< ", vector: " << data.vector_val[0]
					<< ", map: {\"key\": " << data.map_val["key"] << "}" << std::endl;
				std::cout << "Derived::int: " << data.derived_val << std::endl;*/
			} catch(const std::bad_cast&) {
				std::cout << "invalid type cast" << std::endl;
			}
		}
		break;
		default:
		{
			std::cout << "BUG: plz inform to linear-developpers" << std::endl;
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
			std::cout << "BUG: plz inform to linear-developpers" << std::endl;
		}
		break;
		}
	}

private:
	int m_retries;
	linear::Timer m_retryTimer;
};


Client::Client()
{

#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
	m_serverVer = 0;
	m_errIndex = 0;
	m_msgIndex = 0;

	m_handler = linear::shared_ptr<Handler>(new Handler());
	m_client = linear::TCPClient(m_handler);

	m_timeout = 10000;
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

	if (!IsConnected()) return errCONNECT;

	COMERROR ret = errOK;
/*
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
	*/
    return ret;
}


//void(rpc::client &, connection_state, connection_state)
/*
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
*/



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
		addr = "localhost";
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

/*	if (!m_connected) return -1;
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
	}CATCHRPC;*/
	return -1;
}

bool Client::IsBusy(const int state)
{
	if (!IsConnected()) return true;
	if(m_statusCache > m_state.machineStatus)
	{
		return(m_statusCache > state);
	}
	return (m_state.machineStatus > state);
}

void Client::DrivesOn(const bool state)
{
	linear::Notify notify("DrivesOn", state);
	notify.Send(m_socket);
}

void Client::JogVel(const Axes& velocities)
{
	linear::Notify notify("JogVel", velocities);
	if (notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcJOGGING);
	}
}

bool Client::Notify(std::string function, const linear::type::any& param)
{
	linear::Notify notify(function, param);
	return (notify.Send(m_socket) == linear::LNR_OK);
}


bool Client::Call(std::string function, const linear::type::any& param, linear::Response& response)
{
	linear::Request request(function, param);
	ResponseTrack trk(this, request.msgid, response);
	if (request.Send(m_socket, m_timeout, Client::ResponseTrack::OnResponse, Client::ResponseTrack::OnError) != linear::LNR_OK)
	{
		return false;
	}
	return trk.Wait();
}


bool Client::Mdi(const string line)
{
	linear::Response response;
	if (!Call("Mdi", line, response)) return false;
	SetBusy(mcMDI);
	return response.as<BoolData>().value;
}


void Client::FeedOverride(const double percent)
{
	linear::Notify notify("FeedOverride", percent);
	notify.Send(m_socket);
}

void Client::SpindleOverride(const double percent)
{
	linear::Notify notify("SpindleOverride", percent);
	notify.Send(m_socket);
}

void Client::RapidOverride(const double percent)
{
	linear::Notify notify("RapidOverride", percent);
	notify.Send(m_socket);
}


bool Client::LoadFile(string file)
{
	if (!IsConnected()) return false;
	linear::Response response;
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
		if (!Call("SendInit", name, response)) return false;
		file = response.as<string>();
		if (file.empty()) return false;
		char buf[FILE_BLOCK_SIZE];
		int bytes = 0;
		FileData dat;
		dat.block = 0;

		do
		{
			bytes = fread(buf, 1, FILE_BLOCK_SIZE, fp);
			dat.data.assign(buf, bytes);

			if (!Call("SendData", dat, response)) return false;
			if (!response.as<BoolData>().value) return false;
			Poll();
		} while (bytes == FILE_BLOCK_SIZE);
	}


	bool ret = false;
	Call("LoadFile", file, response);
	ret = response.as<BoolData>().value;
	return ret;
}

bool Client::CloseFile()
{
	linear::Response response;
	if (!Call("CloseFile",0, response)) return false;
	bool ret = false;
	ret = response.as<BoolData>().value;
	return ret;
}


void Client::CycleStart()
{
	linear::Notify notify("CycleStart", true);
	if (notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
}

void Client::CycleStop()
{
	linear::Notify notify("CycleStart", false);
	notify.Send(m_socket);
}

void Client::FeedHold(const bool state)
{
	linear::Notify notify("FeedHold", state);
	notify.Send(m_socket);
}


void Client::BlockDelete(const bool state)
{
	linear::Notify notify("BlockDelete", state);
	notify.Send(m_socket);
}

void Client::SingleStep(const bool state)
{
	linear::Notify notify("SingleStep", state);
	notify.Send(m_socket);
}

void Client::OptionalStop(const bool state)
{
	linear::Notify notify("OptionalStop", state);
	notify.Send(m_socket);
}

void Client::Home(const unsigned int axis)
{
	if (axis >= MAX_AXES) return;
	BoolAxes axes;
	axes.Zero();
	axes.array[axis] = true;
	linear::Notify notify("Home", axes);
	if(notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
}

void Client::HomeAll()
{
	
	BoolAxes axes;
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		axes.array[ct] = true;
	}
	linear::Notify notify("Home", axes);
	if(notify.Send(m_socket) == linear::LNR_OK)
	{
		SetBusy(mcRUNNING);
	}
}

string Client::GetNextError()
{
	if (m_errIndex >= m_state.errorCount)
	{
		return string();
	}

	linear::Response response;
	Call("GetError", m_errIndex++, response);
	return response.as<string>();
}

string Client::GetNextMessage()
{
	if (m_msgIndex >= m_state.messageCount)
	{
		return string();
	}

	linear::Response response;
	Call("GetMessage", m_msgIndex++, response);
	return response.as<string>();
}



bool Client::IsLocal()
{
	const linear::Addrinfo& peer = m_socket.GetPeerInfo();
	const linear::Addrinfo& self = m_socket.GetSelfInfo();
	return peer.addr == self.addr;
}

} //namespace CncRemote
