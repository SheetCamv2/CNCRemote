/****************************************************************
CNCRemote server
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




#ifndef CNCSERVER_H_INCLUDED
#ifndef CNCSERVER_H_INCLUDED

#include "cnccomms.h"
#include "cncstatebuf.pb.h"

namespace CncRemote {

class Server;

class Connection : public Comms
{
public:

    Connection(CActiveSocket * socket, Server * server);
    virtual ~Connection();
    void Close();
    COMERROR Poll(); //Call at least as often as you need to send/receive data. Only needed if this connection isn't running in it's own thread
    virtual void UpdateState(); //Called just before state is sent out. Default code just grabs state from the server and sends it out. Override this if you need to add your own state data.
    inline bool IsClosing(){return m_closing;};
    COMERROR Run(); //Start this connection in a new thread. Returns immediately after the thread has been created.

protected:
	void RemoveTemp();
	void RecieveFileInit(const CmdBuf& cmd); //Helper to handle cmdSENDFILEINIT
	void RecieveFileData(const CmdBuf& cmd); //Helper to handle cmdSENDFILEDATA


    string m_curId;
    CActiveSocket * m_socket;
    Server * m_server;
	FILE * m_loadFile;
	int m_loadLength;
	int m_loadCount;
	CncString m_tempFileName;

private:
#ifdef _WIN32
    static DWORD WINAPI Entry( LPVOID param );
#else
    static void * Entry(void * param);
#endif // _WIN32
    void Entry();

    bool m_closing;
#ifdef _WIN32
    HANDLE m_thread;
#else
    pthread_t m_thread;
#endif
};

//simple class to handle locking. Mutex remains locked for the lifetime of this object.
class MutexLocker
{
public:
    MutexLocker(MUTEX * mutex)
    {
        m_mutex = mutex;
        MUTEX_LOCK(mutex);
    }

    ~MutexLocker()
    {
        MUTEX_UNLOCK(m_mutex);
    }

private:
    MUTEX * m_mutex;
};

class Server
{
public:
    Server();
    virtual ~Server();
    void SetTimeout(const float seconds);
    COMERROR Bind(const uint32_t port = DEFAULT_COMMS_PORT);
    COMERROR Poll();
    void RemoveConn(Connection * conn);
    virtual Connection * CreateConnection(CActiveSocket * client, Server * server)= 0;
    Packet GetState();
    virtual void UpdateState(){};
    MutexLocker GetLock() {return (MutexLocker(&m_syncLock));} //Sync your thread to the main thread for as long as the MutexLocker object exists. Basically locks your thread to the server's Poll() loop

protected:
    void SetTimeout();

    CPassiveSocket * m_socket;
    float m_timeout;
    bool m_listening;
    vector<Connection *> m_conns;
    StateBuf m_state;
    Packet m_packet;
    Packet m_statePacket;
    MUTEX m_stateLock;
    MUTEX m_syncLock;
	int32_t m_heartBeat;
private:

};

}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED

