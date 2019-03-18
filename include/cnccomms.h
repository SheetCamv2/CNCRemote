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
#include "linear/message.h"
#include <math.h>

using namespace std;


namespace CncRemote
{

#define CNCREMOTE_PROTOCOL_VERSION 3.0f
#define CNCREMOTE_MIN_PROTOCOL_VERSION 3.0f


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
		mcMOVING, ///<Machine is moving but not executing code (homing/jogging etc)
		mcMDI, ///<Executing MDI code
		mcRUNNING, ///<Executing g-code file
	};

	enum PREVIEWTYPE {
		prevSTART, // Start point
		prevMOVE, // Cutting move
		prevRAPID, // Rapid move
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

		double Length() { return (sqrt((x * x) + (y * y) + (z * z) + (a * a) + (b * b) + (c * c))); }
	};

	struct PreviewAxes : Axes
	{
		union {
			PREVIEWTYPE type;
			int _type; //MessagePack can't handle enums directly
		};
	};

	typedef std::vector<PreviewAxes> PreviewData;

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


	class State {
	public:
		State(); ///<Create a standard instance

		//All memory from here to the start of classes (see near the end of this class) will be
		//initialised to 0 when this class is instantiated.

		Axes position; ///<Axes in tool coordinates (in metric units)
		Axes machinePos; ///<Axes in machine coordinates (in metric units)
		bool feedHold; ///<Feed hold status
		double feedOverride; ///<Feed rate override percentage. 1 = 100%
		//    bool control_on = 7; //Control is on and ready to execute command
		bool optionalStop; ///<stop
		bool blockDelete; ///<block delete
		union {
			CONTROLSTATE machineState; ///<Current machine state
			int _machineState; //MessagePack can't handle enums directly
		};
		int currentLine; ///<current gcode line running. -1 if no line is running
		bool singleStep; ///<Single step
		double spindleCmd; ///<commanded spindle speed
		double spindleActual; ///<actual spindle speed
		union {
			SPINDLESTATE spindleState; ///<Current spindle status
			int _spindleState;  //MessagePack can't handle enums dirtectly
		};
		bool mist; ///<Mist coolant status
		bool flood; ///<Flood coolant status
		BoolAxes homed; ///<homed status
		BoolAxes axisAngular; ///<true if the axis is angular
		double maxFeedLin; ///<Maximum linear feed rate
		double maxFeedAng; ///<Maximum angular feed rate
		double gcodeUnits; ///<Convert mm to the currently selected g-code linear units Usually 1 for metric, 1/25.4 for inch
		double spindleOverride; ///<Spindle override percentage. 1 = 100%
		double rapidOverride; ///<Rapid override percentage. 1 = 100%
		double feedCmd; ///<Commanded feed rate
		double feedActual; ///<Actual feed rate
		int tool; ///<Currently selected tool

		// The following are for internal use
		int errorCount; ///<The total number error messages
		int messageCount; ///<The total number warning messages
		int fileCount; ///<A count of files loaded. Used to indicate that a file has changed



        // Classes go here
		string interpState; ///<Interpreter state text



		MSGPACK_DEFINE_MAP(machinePos, position, feedOverride, feedHold, optionalStop, blockDelete, _machineState,
			currentLine, singleStep, spindleCmd, spindleActual, _spindleState, mist, flood, homed, axisAngular, errorCount, messageCount,
			maxFeedLin, maxFeedAng, gcodeUnits, spindleOverride, rapidOverride, feedCmd, feedActual, tool, interpState, fileCount);

	};


	struct ExceptionData
	{
		ExceptionData() {}
		ExceptionData(string m, string f) { message = m; function = f; }
		string message;
		string function;
		MSGPACK_DEFINE_MAP(message, function);
	};



	template <class A1, class A2>
	struct CallData2
	{
		A1 arg1;
		A2 arg2;
		MSGPACK_DEFINE_MAP(arg1, arg2);
	};


#define CONN_TIMEOUT (2000000) //2 seconds



#define DEFAULT_COMMS_PORT 5090

//class Server;

enum COMERROR
{
    errOK, //no error
    errCONNECT, //No connection
    errFAILED, //Failed to send data
    errNODATA, //no data was received. This is not an error. Simply no data was available to be processed.
    errTHREAD, //Failed to create thread
    errRUNNING, //Thread is already running
    errNOTHREAD, //Unable to create thread
	errWRONGVER, //Server uses old protocol version
};

enum CONNSTATE
{
    connNONE, //No network connection
    connDATA, //Connected and talking
};

} //namespace CncRemote


#endif // Comms_H
