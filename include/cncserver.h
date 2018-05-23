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
#include "rpc/server.h"

namespace CncRemote {

class Server;

#ifdef _WIN32
#define MUTEX HANDLE
#define MUTEX_LOCK(mutex) WaitForSingleObject(mutex, INFINITE)
#define MUTEX_UNLOCK(mutex) ReleaseMutex(mutex)
#define MUTEX_CREATE(mutex) mutex = CreateMutex(NULL, false, NULL);
#define MUTEX_DESTROY(mutex) CloseHandle(mutex);
#else
#define MUTEX pthread_mutex_t
#define MUTEX_LOCK(mutex) pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&mutex)
#define MUTEX_CREATE(mutex) pthread_mutex_init(&mutex, NULL)
#define MUTEX_DESTROY(mutex) pthread_mutex_destroy(mutex);
#endif // _WIN32

//simple class to handle locking. Mutex remains locked for the lifetime of this object.
class MutexLocker
{
public:
    MutexLocker(MUTEX * mutex)
    {
        m_mutex = mutex;
        MUTEX_LOCK(*mutex);
    }

    ~MutexLocker()
    {
        MUTEX_UNLOCK(*m_mutex);
    }

private:
    MUTEX * m_mutex;
};

class Server
{
public:
    Server();
    virtual ~Server();
//    void SetTimeout(const float seconds);
    COMERROR Bind(const uint32_t port = DEFAULT_COMMS_PORT);
    COMERROR Poll();
//    void RemoveConn(Connection * conn);
//    virtual Connection * CreateConnection(CActiveSocket * client, Server * server)= 0;
    virtual void UpdateState(){};
    MutexLocker GetLock() {return (MutexLocker(&m_syncLock));} //Sync your thread to the main thread for as long as the MutexLocker object exists. Basically locks your thread to the server's Poll() loop

protected:
	virtual State GetState() = 0;
	virtual void DrivesOn(const bool state) = 0;
	virtual void JogVel(const Axes velocities) = 0;
	virtual bool Mdi(const string line) = 0;
	virtual void SpindleOverride(const double percent) = 0;
	virtual void FeedOverride(const double percent) = 0;
	virtual void RapidOverride(const double percent) = 0;
	virtual bool LoadFile(const string file) = 0;
	virtual void CloseFile() = 0;
	virtual void CycleStart() = 0;
	virtual void CycleStop() = 0;
	virtual void FeedHold(const bool state) = 0;
	virtual void BlockDelete(const bool state) = 0;
	virtual void SingleStep(const bool state) = 0;
	virtual void OptionalStop(const bool state) = 0;
	virtual void Home(const BoolAxes axes) = 0;

    MUTEX m_syncLock;

	rpc::server* m_server;
//	int32_t m_heartBeat;
private:

};

}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED

