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

#include <time.h>
#include "cncplugin.h"
#include "timer.h"
#include "rpc/client.h"
//#include "rpc/server.h"

using namespace std;

namespace CncRemote
{

#define PROTOCOL_VERSION 1.0 //Current protocol version
#define MIN_PROTOCOL_VERSION 1.0 //Minimum protocol version that is compatible

#define MAX_AXES 6

	enum SPINDLESTATE {
		spinOFF,
		spinFWD,
		spinREV,
	};

	enum CONTROLSTATE {
		mcNO_SERVER, //Server is not responding. Note your server should never use this value.
		mcOFFLINE, //Machine is off line (e.g server is running but it can't connect to the machine hardware)
		mcOFF, //Machine is connected but drives are off
		mcIDLE, //Drives on but not moving
		mcJOGGING, //Machine is jogging
		mcMDI, //Executing MDI code
		mcRUNNING, //Executing g-code file
	};


	struct Axes{

		union {
			double array[MAX_AXES];
			struct {
				double x;
				double y;
				double z;
				union {
					double a;
					double u;
				};
				union {
					double b;
					double v;
				};
				union {
					double c;
					double w;
				};
			};
		};
		MSGPACK_DEFINE_ARRAY(array);

		void Zero() { memset(array, 0, sizeof(array)); }

		
		#define OP(n) Axes& operator n (const double src)\
		{\
			for (int ct = 0; ct < MAX_AXES; ct++)\
			{\
				array[ct] n src;\
			}\
			return *this;\
		}\
		Axes& operator n (const Axes& src)\
		{\
			for (int ct = 0; ct < MAX_AXES; ct++)\
			{\
				array[ct] n src.array[ct];\
			}\
			return *this;\
		}

				OP(+=);
				OP(-=);
				OP(*=);
				OP(/=);
		#undef OP

		#define OP(n) Axes operator n (const double src)\
		{\
			Axes ret;\
			for (int ct = 0; ct < MAX_AXES; ct++)\
			{\
				ret.array[ct] = array[ct] n src;\
			}\
			return ret;\
		}\
		Axes operator n (const Axes& src)\
		{\
			Axes ret;\
			for (int ct = 0; ct < MAX_AXES; ct++)\
			{\
				ret.array[ct] = array[ct] n src.array[ct];\
			}\
			return ret;\
		}
				OP(+);
				OP(-);
				OP(*);
				OP(/);
		#undef OP



	};

	struct BoolAxes {
		union {
			bool array[MAX_AXES];
			struct {
				bool x;
				bool y;
				bool z;
				union {
					bool a;
					bool u;
				};
				union {
					bool b;
					bool v;
				};
				union {
					bool c;
					bool w;
				};
			};
		};
		MSGPACK_DEFINE_ARRAY(array);

		void Zero() { memset(array, 0, sizeof(array)); }
	};


	struct State {
		Axes absPos; //Axes in machine coordinates (in metric units)
		Axes offsetWork; //Work offsets (in metric units)
		Axes offsetFixture; //Fixture offsets (in metric units)
		bool feedHold; //Feed hold status
		double feedOverride; //Feed rate override percentage. 1 = 100%
		//    bool control_on = 7; //Control is on and ready to execute command
		bool optionalStop; //stop
		bool blockDelete; //block delete
		union {
			CONTROLSTATE machineStatus; //Current machine state
			int _machineStatus; //MessagePack can't handle enums dirtectly
		};
		int currentLine; //current gcode line running. -1 if no line is running
		bool singleStep; //Single step
		double spindleSpeed; //commanded spindle speed
		union {
			SPINDLESTATE spindleState; //Current spindle status
			int _spindleState;  //MessagePack can't handle enums dirtectly
		};
		bool mist; //Mist coolant status
		bool flood; //Flood coolant status
		BoolAxes homed; //homed status
		BoolAxes axisLinear; //true if the axis is linear
		string errorMsg; //Contains last error message
		string displayMsg; //Contains any text that should be displayed to the user
		double maxFeedLin;
		double maxFeedAng;
		double gcodeUnits; //Convert mm to the currently selected g-code linear units Usually 1 for metric, 1/25.4 for inch
		double spindleOverride; //Spindle override percentage. 1 = 100%
		double rapidOverride; //Rapid override percentage. 1 = 100%

		MSGPACK_DEFINE_MAP(absPos, offsetWork, offsetFixture, feedOverride, feedHold, optionalStop, blockDelete, _machineStatus,
			currentLine, singleStep, spindleSpeed, _spindleState, mist, flood, homed, axisLinear, errorMsg, displayMsg,
			maxFeedLin, maxFeedAng, gcodeUnits, spindleOverride, rapidOverride);

		void Clear() { memset(this, 0, sizeof(State));}
	};



#define CONN_TIMEOUT (2000000) //2 seconds



#define DEFAULT_COMMS_PORT 5090

class Server;

enum COMERROR
{
    errOK, //no error
//    errSOCKET, //Failed to create socket
 //   errBIND, //Failed to bind socket (server only)
    errCONNECT, //No connection
 //   errNOSOCKET, //No valid socket
    errFAILED, //Failed to send data
    errNODATA, //no data was received. This is not an error. Simply no data was available to be processed.
    errTHREAD, //Failed to create thread
    errRUNNING, //Thread is already running
    errNOTHREAD, //Unable to create thread
};

enum CONNSTATE
{
    connNONE, //No network connection
 //   connNETWORK, //Network connection but no data is being trasferred
    connDATA, //Connected and talking
};

#if 0
class Comms
{
public:
//    Comms(CActiveSocket *socket, Server * server = NULL);
    Comms();
    virtual ~Comms();

//	bool IsLocal(); //Returns true if the connection is to the local host

/*    void SetSocket(CActiveSocket *socket)
    {
        m_socket = socket;
    }*/
//    void Connect(const CncString& address, const uint32_t port); //Connect to a server. Note this is asynchronous so the actual connection may take place later
/*    CONNSTATE IsConnected()
    {
        return m_connState;   //Is the server connected and running?
    }*/
//    virtual void OnConnection(const CONNSTATE state) {}; //Override this if you want to be notfied of changes to the connection.

    void Close(); //Close the connection

protected:

    static void * Entry(void * t);
    void * Entry();
/*    void CheckConn();
    int RecvString(string& data);
    bool SendPacket(const Packet &packet);
    size_t CobsEncode(const uint8_t *ptr, size_t length, uint8_t *dst);
    void CobsDecode(const uint8_t *ptr, size_t length);
    void OnData();

    virtual void HandlePacket(const Packet & pkt) = 0;
    virtual void OnPacketError() {};

    Packet m_packet;*/
private:
//    CActiveSocket *m_socket;
 //   uint32_t m_port;
 //   CncString m_address;

//    void Connected(const CONNSTATE state);
/*    CONNSTATE m_connState;
    float m_socketTimeout;
    UTimer m_connTimer;
    uint64_t m_connTime;
    string m_rxData;*/

};
#endif
} //namespace CncRemote


#endif // Comms_H
