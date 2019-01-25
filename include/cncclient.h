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


#ifndef CNCCLIENT_H_INCLUDED
#define CNCCLIENT_H_INCLUDED

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "cnccomms.h"
#include "cncplugin.h"
#include "linear/tcp_client.h"
#include "linear/log.h"
#include "linear/timer.h"
#include "linear/handler.h"
#include "linear/mutex.h"


namespace CncRemote
{
#ifdef USE_PLUGINS
struct Plugin
{
    LIBHANDLE handle;
    CNCSTARTFUNC Start;
    CNCSTOPFUNC Stop;
    CNCGETNAMEFUNC GetName;
    CNCQUITFUNC Quit;
    CNCPOLLFUNC Poll;
    CNCCONTROLEXISTSFUNC ControlExists;
};

class Client : private Plugin
#else
class Client
#endif // USE_PLUGINS

{
public:
    Client();
	virtual ~Client();
#ifdef USE_PLUGINS
	bool LoadPlugins(const CncString& pluginPath); ///<Load plugins from the given path. Note path should end with a trailing path delimiter. Returns false if plugins dir nor found.
	const vector<Plugin> GetPlugins(){return m_plugins;} ///<Get the available plugins
#endif
    COMERROR Poll(); ///<Call at least as often as you need to send/receive data. Returns true if any data was received.
	bool Connect(const unsigned int index, const CncString& address, const uint32_t port); ///<Connect to server. if index == 0, use a remote server else use the selected plugin (1 = first plugin and so on)
	void Disconnect(); ///<Disconnect (normally disconnection is automatic so you shouldn't need to call this)
 	float Ping(int waitMs); ///<Ping the server. If there is no response within waitMs milliseconds it returns false. Returns time for round trip in milliseconds. Blocks for up to waitMs milliseconds
	bool IsBusy(const int state); ///<Returns true if the machine state is greater then the given state
	bool IsConnected() { return m_socket.GetState() == linear::Socket::CONNECTED; } ///<Returns true if we are connected to server


    void DrivesOn(const bool state); ///<Turn drives/control on
    void JogVel(const Axes& velocities); ///<jog axes at the given rates. Use axis values of 0 to stop jogging.
    bool Mdi(const string line); ///<execute MDI command. Returns false if failed. This function blocks until the MDI code is sent.
    void FeedOverride(const double percent); ///<set feed rate override (1 = 100%)
    void SpindleOverride(const double percent); ///<set spindle override (1 = 100%)
    void RapidOverride(const double percent); ///<set rapid override (1 = 100%)
	/**load the file.
	If the server is remote the file is first uploaded to a temporary directory.
	Returns false if failed. This function blocks until the file is loaded.
	*/
	bool LoadFile(string file); 
	bool CloseFile(); ///<Close any loaded files (some controls lock the file they have open)
    void CycleStart(); ///<cycle start
    void CycleStop(); ///<Stop execution
    void FeedHold(const bool state); ///<pause/resume motion
    void BlockDelete(const bool state); ///<Turn block delete on/off
    void SingleStep(const bool state); ///<Turn single stepping on/off
    void OptionalStop(const bool state); ///<Turn optional stop stepping on/off
	void Home(const unsigned int axis); ///<Home a single axis
	void HomeAll(); ///<Home all

	State & GetState() { return (m_state); } ///<Get the current machine state

	bool HasErrors() { return m_errIndex < m_state.errorCount; } ///<Returns true is there are any errors pending
	bool HasMessages() { return m_msgIndex < m_state.messageCount; } ///<Returns true if there are any messages pending
	string GetNextError(); ///<Get the next error. Returns empty string if none pending.
	string GetNextMessage(); ///<Get the next message. Returns empty string if none pending.

	bool IsLocal(); ///<Returns true if this is a local connection
protected:
	void SetBusy(const int state); ///<Must be called if the last command sent may make the machine move
	State m_state;
	int m_roundTrip; //Time for a GetState() round trip in us

	bool Notify(std::string function, const linear::type::any& param);
	bool Call(std::string function, const linear::type::any& param, linear::Response& response);
private:


#ifdef USE_PLUGINS
    vector<Plugin> m_plugins;
    Plugin * m_plugin;
#endif
	int m_statusCache;
	int32_t m_busyHeart;
	int32_t m_heartBeat;
	float m_serverVer;
	unsigned int m_errIndex;
	unsigned int m_msgIndex;

	class Handler;
	friend class Handler;
	linear::shared_ptr<Handler> m_handler;
	linear::TCPClient m_client;
	linear::TCPSocket m_socket;
	unsigned m_timeout;
	
	struct ResponseData
	{
		uint32_t msgid;
		Client * client;
		linear::mutex mutex;
		linear::Response* response;
		bool ok;
	};
	
	class ResponseTrack
	{
	public:
		ResponseTrack(Client* client, const int32_t msgid, linear::Response& response);
		~ResponseTrack();
		bool Wait();
		static ResponseData* Find(const int32_t msgid);

		static void OnResponse(const linear::Socket& socket, const linear::Response& response);
		static void OnError(const linear::Socket& socket, const linear::Request& request, const linear::Error& error);


	private:
		ResponseData m_data;
		static std::vector<ResponseData *> m_responses;
		static linear::mutex m_mutex;
	};
};

} //namespace CncRemote

#endif //#ifndef CNCCLIENT_H_INCLUDED


