/****************************************************************
CNCRemote communications
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




#ifndef Comms_H
#define Comms_H
#if defined(_WIN32) | defined(_WIN64)
#include <winsock2.h>
#endif

#include <time.h>
#include "cncstatebuf.pb.h"
#include "cncplugin.h"
#include "PassiveSocket.h"
#include "timer.h"

#ifdef _WIN32
#define MUTEX HANDLE
#define MUTEX_LOCK(mutex) WaitForSingleObject(mutex, INFINITE)
#define MUTEX_UNLOCK(mutex) ReleaseMutex(mutex)
#define MUTEX_CREATE(mutex) mutex = CreateMutex(NULL, false, NULL);
#else
#define MUTEX pthread_mutex_t
#define MUTEX_LOCK(mutex) pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&mutex)
#define MUTEX_CREATE(mutex) pthread_mutex_init(&mutex, NULL)
#endif // _WIN32

using namespace std;

namespace CncRemote
{
#define CONN_TIMEOUT (2000000) //2 seconds

#define MAX_PACKET_SIZE 2048

#define MAX_AXES 6

#define pktESCAPE 0xFF

#define DEFAULT_COMMS_PORT 5080

class Server;

#ifdef __GNUC__
#define PACKED( class_to_pack ) class_to_pack __attribute__((__packed__));
#else
#define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#endif

struct Packet
{
    PACKED(
	struct
	{
		uint16_t cmd;
		uint16_t heartBeat;
	} hdr;
	)
    string data;
};

enum COMERROR
{
    errOK, //no error
    errSOCKET, //Failed to create socket
    errBIND, //Failed to bind socket (server only)
    errCONNECT, //Lost connection
    errNOSOCKET, //No valid socket
    errFAILED, //Failed to send data
    errNODATA, //no data was received. This is not an error. Simply no data was available to be processed.
    errTHREAD, //Failed to create thread
    errRUNNING, //Thread is already running
    errNOTHREAD, //Unable to create thread
};

enum CONNSTATE
{
    connNONE, //No network connection
    connNETWORK, //Network connection but no data is being trasferred
    connDATA, //Connected and talking
};


class Comms
{
public:
    Comms(CActiveSocket *socket, Server * server = NULL);
    Comms();
    virtual ~Comms();

	bool IsLocal(); //Returns true if the connection is to the local host

    void SetSocket(CActiveSocket *socket)
    {
        m_socket = socket;
    }
    void Connect(const CncString& address, const uint32_t port); //Connect to a server. Note the connection will only be attempted on the next Poll()
    CONNSTATE IsConnected()
    {
        return m_connState;   //Is the server connected and running?
    }
    virtual void OnConnection(const CONNSTATE state) {}; //Override this if you want to be notfied of changes to the connection.


    COMERROR Poll(); //Call on a regular basis
    void SetTimeout(float seconds); //Set timeout for data send/receive. If set to 0 send/receive are non-blocking

    void Close(); //Close the connection

    enum CMDTYPE   //it is safe to add to this list but entries must not be deleted or rearranged
    {
        cmdNULL,
        cmdPING,	//none
        cmdSTATE,	//StateBuf
        cmdDRIVES_ON, //boolean
        cmdJOG_VEL,	//Axes. Jog at given velocities
        cmdJOG_STEP,  //Axes. Jog the given amount
        cmdMDI,		//string
        cmdFRO,		//float
        cmdFILE,	//string
        cmdCLOSE_FILE, //none
        cmdSTART,	//none
        cmdSTOP,	//none
        cmdFEED_HOLD,	//none
        cmdBLOCK_DEL,	//boolean
        cmdSINGLE_STEP,	//boolean
        cmdOPT_STOP,		//boolean
        cmdSEND_FILE_INIT,	//string, intval. String is file name (note: just the file name excluding path), intval is the length
        cmdSEND_FILE_DATA,	//raw file data in packets of up to 1024 bytes
        cmdSPINDLE, //integer: one of spinOFF,spinFWD,spinREV
        cmdHOME,	//BoolAxes. All true = home all. otherwise ONE axis only.
		cmdSPINDLE_OVER, //float
		cmdRAPID_OVER, //float


        cmdMAX, //this should always be the last entry. Note your code should always ignore command numbers past this.
    };

protected:

    static void * Entry(void * t);
    void * Entry();
    void CheckConn();
    int RecvString(string& data);
    bool SendPacket(const Packet &packet);
    size_t CobsEncode(const uint8_t *ptr, size_t length, uint8_t *dst);
    void CobsDecode(const uint8_t *ptr, size_t length);
    void OnData();

    virtual void HandlePacket(const Packet & pkt) = 0;
    virtual void OnPacketError() {};

    Packet m_packet;
private:
    CActiveSocket *m_socket;
    uint32_t m_port;
    CncString m_address;

    void Connected(const CONNSTATE state);
    CONNSTATE m_connState;
    float m_socketTimeout;
    UTimer m_connTimer;
    uint64_t m_connTime;
    string m_rxData;

};

} //namespace CncRemote


#endif // Comms_H
