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
#include <mutex>


namespace CncRemote
{

class TimeoutError : public std::runtime_error
{
public:
	TimeoutError() :std::runtime_error("Timed out calling function") {}
};

class RemoteError : public std::runtime_error
{
public:
	RemoteError(string err) : std::runtime_error(err) {}

};

class TransferError : public std::runtime_error
{
public:
	TransferError(std::string err) : runtime_error(err) {}
};

class SendError : public std::runtime_error
{
public:
	SendError(string err) : runtime_error(err) {}
};


class BusyError : public std::runtime_error
{
public:
	BusyError() :std::runtime_error("Waiting for response") {}
};

/**
This class handles RPC call data.
NOTE: when using asynchronous calls this object must remain in scope until the asynchronous call returns or throws an error.
*/

class RemoteCall
{
public:
	RemoteCall();
	/**
	Call a remote function and wait for the result.
	The timeout is in ms.
	Returns the response from the call. Throws an exception on any errors.
	*/
	linear::Response& Call(linear::Socket& socket, unsigned timeout, const std::string& function)
	{
		return Call(socket, timeout, function, 0);
	}

	linear::Response& Call(linear::Socket& socket, unsigned timeout, const std::string& function, const linear::type::any& param);

	template<typename A1, typename A2>
	linear::Response& Call(linear::Socket& socket, unsigned timeout, const std::string& function, A1 arg1, A2 arg2)
	{
		CallData2<A1, A2> dat;
		dat.arg1 = arg1;
		dat.arg2 = arg2;
		return Call(socket, timeout, function, dat);
	}


	/**
	Call a remote function asynchronously.
	The timeout is in ms.
	Returns immediately. You need to wait for IsBusy to return false before attempting to read the response.
	*/
	void CallAsync(linear::Socket& socket, unsigned timeout, const std::string& function)
	{
		CallAsync(socket, timeout, function, 0);
	}
	void CallAsync(linear::Socket& socket, unsigned timeout, const std::string& function, const linear::type::any& param);
	template<typename A1, typename A2>
	void CallAsync(linear::Socket& socket, unsigned timeout, const std::string& function, A1 arg1, A2 arg2)
	{
		CallData2<A1, A2> dat;
		dat.arg1 = arg1;
		dat.arg2 = arg2;
		return CallAsync(socket, timeout, function, dat);
	}


	/**
	Wait for a remote call to complete.
	*/
	bool Wait(unsigned ms = 0);

	/**
	Check if we have a valid response
	*/
	bool HasResponse() { return m_hasResponse; }

	/**
	Get the response. Note only use this after checking HasResponse()
	You need to check if the response contains an error.
	*/
	linear::Response& GetResponse();

	/**
	We are waiting for a response
	*/
	bool IsBusy() { return m_busy; }

	/**
	Clear the HasResponse flag. Useful if re-using this object.
	*/
	void ClearResponse() { m_hasResponse = false;}

private:
	void OnResponse(const linear::Socket& socket, const linear::Response& response);
	void OnError(const linear::Socket& socket, const linear::Request& request, const linear::Error& error);

	linear::Response * m_response;
	bool m_hasResponse;
	string m_error;
	bool m_busy;

	std::function<void(const linear::Socket&, const linear::Response&)> m_onResponse;
	std::function<void(const linear::Socket&, const linear::Request&, const linear::Error&)> m_onError;
};





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
/**
This is the main client class.
*/

class Client
#endif // USE_PLUGINS

{
public:
    Client();
	virtual ~Client();
#ifdef USE_PLUGINS
/**
Load plugins from the given path.
Note path should end with a trailing path delimiter.
Returns false if plugins dir nor found.

If USE_PLUGINS is defined you must implement the function DoLog() as defined in cncplugin.h
*/
	bool LoadPlugins(const CncString& pluginPath, CNCLOGFUNC logFunc);
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
	/** Get a list of the currently active G-codes.
	The g-codes are multiplied by 10. For example:
	\verbatim
	G03 = 30
	G91.1 = 911
	\endverbatim
	*/
	vector<int> GetGCodes();

	/**Async version of GetGCodes
	Example code:
	\verbatim
	CncRemote::RemoteCall call;
	GetGCodes(call);
	...do stuff
	if(call.HasResponse())
	{
		try
		{
			vector<int> result = call.GetResponse().result.as<vector<int>>();
			//do something with the result

		}
		catch (std::exception& exc) //catch any exceptions
		{
		}
	}
	\endverbatim
	*/
	void GetGCodes(RemoteCall& call);

	vector<int> GetMCodes(); ///<Get a list of the currently active M-codes.
	void GetMCodes(RemoteCall& call); ///<Async version of GetMCodes

	bool HasErrors() { return m_errIndex < m_state.errorCount; } ///<Returns true is there are any errors pending
	bool HasMessages() { return m_msgIndex < m_state.messageCount; } ///<Returns true if there are any messages pending
	string GetNextError(); ///<Get the next error. Returns empty string if none pending.
	string GetNextMessage(); ///<Get the next message. Returns empty string if none pending.

	bool IsLocal(); ///<Returns true if this is a local connection

#ifdef HANDLE_CNCREMOTE_EXCEPTIONS
	/**
	If HANDLE_CNCREMOTE_EXCEPTIONS is defined all communication exceptions are rerouted to this handler.
	Otherwise most of the above calls will throw exceptions if there are any errors.
	*/
	virtual void OnException(std::exception& exc) = 0;
#endif

	virtual void OnConnect() {} ///<Called when we are connected to the server
	virtual void OnDisConnect() {} ///<Called when we are disconnected from the server
	virtual void OnIncorrectVersion(const float serverVersion) {} ///<Called if the server is too old
	virtual void OnRemoteException(const ExceptionData& exception) {} ///<Called if there is an exception on the server after a notify is sent


protected:

	void SetBusy(const int state); ///<Must be called if the last command sent may make the machine move
	State m_state;
	int m_roundTrip; ///<Time for a GetState() round trip in us

private:
	void OnDisConnect2(); //Triggered when we are disconnected from the server
	std::chrono::high_resolution_clock::time_point m_pollTimer;

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
	RemoteCall m_StatusCall;
};

} //namespace CncRemote

#endif //#ifndef CNCCLIENT_H_INCLUDED


