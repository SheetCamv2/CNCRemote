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

/** simple class to handle locking. Mutex remains locked for the lifetime of this object.
*/
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

/** The server base class.
All servers are based on this class. 
*/
class Server
{
public:
    Server();
    virtual ~Server();
	/**	Bind a given port to receive packets.
	*/
	COMERROR Bind(const uint32_t port = DEFAULT_COMMS_PORT);
	/**	Call this every so often from your main loop.
	Note this is used to synchronize connection threads with your main thread.
	If you use thread synchronization you need to call this often to keep latency low.
	*/
    COMERROR Poll();
//    virtual void UpdateState(){};
	/**Sync your thread to the main thread for as long as the MutexLocker object exists. Basically locks your thread to the server's Poll() loop
	*/
	MutexLocker GetLock() {return (MutexLocker(&m_syncLock));} 

protected:
	//Override these to provide machine functionality

	/** Return a State buffer containing the current machine state.
	*/
	virtual State GetState() = 0;

	/** Turn the drives on if you can.
	*/
	virtual void DrivesOn(const bool state) = 0;

	/** Jog axes at the given velocities.
	For each axis 1 = 100%
	*/
	virtual void JogVel(const Axes velocities) = 0;

	/** Execute an MDI string if possible.
	Return false if the string could not be executed.
	Note: Return immediately. Do not wait for MDI to finish.
	*/
	virtual bool Mdi(const string line) = 0;

	/** Set the spindle override (1 = 100%).
	*/
	virtual void SpindleOverride(const double percent) = 0;

	/** Set the feed rate override (1 = 100%).
	*/
	virtual void FeedOverride(const double percent) = 0;

	/** Set the rapid override (1 = 100%).
	*/
	virtual void RapidOverride(const double percent) = 0;

	/** Load a file.
	fileName should be a local file name.
	Return false if the file could not be loaded (file does not exist, machine busy etc).
	*/
	virtual bool LoadFile(const string fileName) = 0;

	/** Close any loaded file.
	Return false if the loaded file cannot be closed (e.g machine is busy).
	Note: return true if no file was loaded.
	*/
	virtual bool CloseFile() = 0;

	/** Cycle start
	*/
	virtual void CycleStart() = 0;

	/** Stop running program/MDI
	*/
	virtual void CycleStop() = 0;

	/** Feed hold
	*/
	virtual void FeedHold(const bool state) = 0;

	/** Enable/disable block delete
	*/
	virtual void BlockDelete(const bool state) = 0;

	/** Enable/disable single step
	*/
	virtual void SingleStep(const bool state) = 0;

	/** Enable/disable optional stop
	*/
	virtual void OptionalStop(const bool state) = 0;

	/** Home the given axes
	*/
	virtual void Home(const BoolAxes axes) = 0;

	/** Start receiving a file.
	Create a temporary file and open it for writing.
	Return the local file name including path. The file name should usually start with nameHint.
	For example if nameHint is "test" the file name could be "\tmp\test001.tmp"
	If the file could not be opened, return an empty string.
	NOTE: nameHint should not contain any characters that are invalid in unix or Windows file names (e.g "\" or "*").

	
	The default behaviour is to open the file in /tmp or %TEMP%
	If you override this and don't override SendData you must set the following:
	m_file = file handle
	m_curBlock = 0
	*/
	virtual string SendInit(string nameHint);

	/** Receive a block of data.
	Each block apart from the last should be FILE_BLOCK_SIZE bytes long.
	block is the block number. Blocks are sent in sequence starting at 0.
	Close the file after receiving a block that is less than FILE_BLOCK_SIZE long.
	Return false and close the file if block numbers are out of sequence or failed to write to disk.

	You should not normally need to override this.
	*/
	virtual bool SendData(const string data, const int block); //You normally don't need to override this


	MUTEX m_syncLock;
	rpc::server* m_server;

	FILE * m_file;
	int m_curBlock;
	string m_curFile;
private:
	void DeleteTemp();


};

}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED

