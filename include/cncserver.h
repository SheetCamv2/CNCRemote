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


/** simple class to handle locking. Mutex remains locked for the lifetime of this object.
*/
class MutexLocker
{
public:
    MutexLocker(Mutex * mutex)
    {
        m_mutex = mutex;
		mutex->Lock();
    }

    ~MutexLocker()
    {
		m_mutex->Unlock();
    }

private:
	Mutex * m_mutex;
};


class LockedState
{
public:
	LockedState(State * state, Mutex * mutex)
	{
		m_state = state;
		m_mutex = mutex;
		mutex->Lock();
	}

	LockedState(LockedState& src)
	{
		m_state = src.m_state;
		m_mutex = src.m_mutex;
		src.m_mutex = NULL;
	}

	virtual ~LockedState()
	{
		if(m_mutex) m_mutex->Unlock();
	}
	State& Data() { return *m_state; }
private:
	State* m_state;
	Mutex * m_mutex;
};


/** The server base class.
All servers are based on this class.
Note the virtual functions are multi threaded.
To make your functions thread safe you must either use GetState() or Sync().
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

	/**Sync your thread to the main thread for as long as the MutexLocker object exists. Basically locks your thread to the server's Poll() loop
	*/
	MutexLocker GetLock() {return (MutexLocker(&m_syncLock));}

	/**Synchronise threads 
	If you are not using GetState, use Sync() to synchronise your thread.
	GetState automatically does this.*/

	#define Sync() MutexLocker _lock = GetLock();


	/** Get the current machine state.
	This is thread safe. Generally you would use this this function as:
	\code
	LockedState lockedState = GetState();
	State& state = lockedState.Data();
	\endcode
	WARNING: You must keep an instance of your locked state for as long as you want to access the data. For instance
	\code
	State& state = GetState().Data();
	\endcode
	is dangerous. You will have a reference to the state but depending on your compiler it may not be thread safe.
	*/
	LockedState GetState();

protected:
	//Override these to provide machine functionality

	/** Called just before m_state is sent to a client.
	*/
	virtual void UpdateState(State& state) = 0;

	/** Turn the drives on if you can.
	*/
	virtual void DrivesOn(const bool state) = 0;

	/** Jog axes at the given velocities.
	For each axis 1 = 100%
	*/
	virtual void JogVel(const Axes velocities) = 0;

	/** Jog axes the given distance at the given velocity.
	A speed of 1 = 100%
	*/
	virtual void JogStep(const Axes distance, const double speed) = 0;

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

	/** Get The given offset
	As different controls handle offsets differently the number of offsets available may vary. As a recommendation:
	\verbatim
	0 = tool offset
	1 = G5x offset
	2 = G9x offset
	\endverbatim
	Return all zeros for indexes that are not recognised.
	*/
	virtual Axes GetOffset(const unsigned int index) = 0;

	/** Start receiving a file.
	Create a temporary file and open it for writing. \n
	Return the local file name including path. The file name should usually start with nameHint.
	For example if nameHint is "test" the file name could be "/tmp/test001.tmp"\n
	If the file could not be opened, return an empty string.
	NOTE: nameHint should not contain any characters that are invalid in unix or Windows file names (e.g "\", "/", "?" or "*").\n


	The default behaviour is to open the file in /tmp or %TEMP%
	If you override this and don't override SendData you must set the following:
	m_file = file handle
	m_curBlock = 0
	*/
	virtual string SendInit(string nameHint);

	/** Receive a block of data.
	Each block apart from the last should be FILE_BLOCK_SIZE bytes long.\n
	block is the block number. Blocks are sent in sequence starting at 0.\n
	Close the file after receiving a block that is less than FILE_BLOCK_SIZE long.\n
	Return false and close the file if block numbers are out of sequence or failed to write to disk.\n

	You should not normally need to override this.
	*/
	virtual bool SendData(const string data, const int block);

    /** Get the error with the given index.
    Valid index numbers are 0 to errorCount-1
	If the index is out of range will return an empty string

	You should not normally need to override this if you use the LogError function.
	*/
    virtual string GetError(const unsigned int index);

	/** Get the message with the given index.
	Valid index numbers are 0 to warningCount-1
	If the index is out of range will return an empty string

	You should not normally need to override this if you use the LogMessage function.
	*/
    virtual string GetMessage(const unsigned int index);

	/** Start graphics preview.
	Return false if cannot preview (file not loaded, preview not implemented etc).
	recommendedSize is a recommended maximum number of points to return. 
	If a preview is currently in progress it should be aborted.
	Note: m_curFile normally contains the path of the currently loaded file (if loaded)
	*/
	virtual bool StartPreview(const int recommendedSize) { return false; }

	/** Get Preview data.
		Return a sequence of PreviewAxes points on the preview path.
		This function will be called multiple times until no more data is available.
		An empty array indicates no more data.
		It is good practice to limit the number of points returned per call to the recommended size in StartPreview.
		Arcs need to be broken into line segments. Try to minimise the number of points returned though.
	*/
	virtual PreviewData GetPreview() { return PreviewData(); }

	/** End preview and close the file if needed. 
	May be called before all preview data has been read, for example if the user aborts previewing.
	*/
	virtual void EndPreview() {}



	/** Log an error message. Empty messages will be discarded.
	*/
	void LogError(string error);

	/** Log a message. Empty messages will be discarded.
	*/
	void LogMessage(string message);

	
	rpc::server* m_server;

	FILE * m_file;
	int m_curBlock;
	string m_curFile;
	vector<string> m_errors;
	vector<string> m_messages;

private:
	Mutex m_syncLock;
	State m_state;

	void DeleteTemp();
	State GetState2();
	bool LoadFile2(const string file);



};

}//namespace CncRemote

#endif

#endif //#ifndef CNCSERVER_H_INCLUDED

