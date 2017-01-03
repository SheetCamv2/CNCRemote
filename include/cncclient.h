#ifndef CNCCLIENT_H_INCLUDED
#define CNCCLIENT_H_INCLUDED

#include "cnccomms.h"
#include "cncplugin.h"

namespace CncRemote
{

struct Plugin
{
	CONTROLSTATUS status;

    LIBHANDLE handle;
    CNCSTARTFUNC Start;
    CNCGETNAMEFUNC GetName;
    CNCQUITFUNC Quit;
    CNCPOLLFUNC Poll;
    CNCCONTROLEXISTSFUNC ControlExists;
};

class Client : public Comms, private Plugin
{
public:
    Client();
	virtual ~Client();
	void LoadPlugins(const CncString& pluginPath); //Load plugins from the given path. Note path should end with a trailing path delimiter.

    bool Poll(); //Call at least as often as you need to send/receive data. Returns true if any data was received.
	const vector<Plugin> GetPlugins(){return m_plugins;} //Get the available plugins
	bool Connect(const int index, const string address); //Connect to server
	void Disconnect(); //Disconnect (normally disconnection is automatic so you shouldn't need to call this)
 	bool IsConnected(){return m_isConnected;} //Is the server connected and running?

    //Avoid adding functions that are specific to a certain control software.
    //If you add functions you must also update Comms::CMDTYPE
    void DrivesOn(const bool state); //Turn drives/control on
    void JogVel(const Axes& velocities); //jog axes at the given rates. Use axis values of 0 to stop jogging.
    void Mdi(const string line); //execute MDI command
    void SetFRO(const double percent); //set feed rate override (1 = 100%)
    void LoadFile(const string file); //load file
	void CloseFile(); //Close any loaded files (some controls lock the file they have open)
    void CycleStart(); //cycle start
    void Stop(); //Stop execution
    void Pause(const bool state); //pause/resume motion
    void BlockDelete(const bool state); //Turn block delete on/off
    void SingleStep(const bool state); //Turn single stepping on/off
    void OptionalStop(const bool state); //Turn optional stop stepping on/off

protected:
	virtual void HandlePacket(const Packet & pkt);

private:
	bool m_isConnected;
	unsigned int m_lastHeart;
	time_t m_nextTime;
    vector<Plugin> m_plugins;
	string m_address;
};

} //namespace CncRemote

#endif //#ifndef CNCCLIENT_H_INCLUDED


