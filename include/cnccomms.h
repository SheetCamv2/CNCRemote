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
#define CNCREMOTE_PROTOCOL_VERSION 2
#define CNCREMOTE_MIN_PROTOCOL_VERSION 2


	enum {
		MAX_AXES = 6, ///< Maximum number of axes
		FILE_BLOCK_SIZE = 16000, ///< Maximum data size when transferring files
	};

	enum SPINDLESTATE {
		spinOFF,
		spinFWD,
		spinREV,
	};

	enum CONTROLSTATE {
		mcNO_SERVER, ///<Server is not responding. Note your server should never use this value.
		mcOFFLINE, ///<Machine is off line (e.g server is running but it can't connect to the machine hardware)
		mcOFF, ///<Machine is connected but drives are off
		mcIDLE, ///<Drives on but not moving
		mcJOGGING, ///<Machine is jogging
		mcMDI, ///<Executing MDI code
		mcRUNNING, ///<Executing g-code file
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
		Axes absPos; ///<Axes in machine coordinates (in metric units)
		Axes offsetWork; ///<Work offsets (in metric units)
		Axes offsetFixture; ///<Fixture offsets (in metric units)
		bool feedHold; ///<Feed hold status
		double feedOverride; ///<Feed rate override percentage. 1 = 100%
		//    bool control_on = 7; //Control is on and ready to execute command
		bool optionalStop; ///<stop
		bool blockDelete; ///<block delete
		union {
			CONTROLSTATE machineStatus; ///<Current machine state
			int _machineStatus; //MessagePack can't handle enums directly
		};
		int currentLine; ///<current gcode line running. -1 if no line is running
		bool singleStep; ///<Single step
		double spindleSpeed; ///<commanded spindle speed
		union {
			SPINDLESTATE spindleState; ///<Current spindle status
			int _spindleState;  //MessagePack can't handle enums dirtectly
		};
		bool mist; ///<Mist coolant status
		bool flood; ///<Flood coolant status
		BoolAxes homed; ///<homed status
		BoolAxes axisLinear; ///<true if the axis is linear
		string errorMsg; ///<Contains last error message
		string displayMsg; ///<Contains any text that should be displayed to the user
		double maxFeedLin;
		double maxFeedAng;
		double gcodeUnits; ///<Convert mm to the currently selected g-code linear units Usually 1 for metric, 1/25.4 for inch
		double spindleOverride; ///<Spindle override percentage. 1 = 100%
		double rapidOverride; ///<Rapid override percentage. 1 = 100%

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

} //namespace CncRemote


#endif // Comms_H
