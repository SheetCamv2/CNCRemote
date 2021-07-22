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


#ifdef _WIN32
#include <WinSock2.h>
#endif


#include "cnccomms.h"
#include <stdio.h>
#include <string.h>

#define CONN_RETRY_START  5000 //first connection retry after 5ms
#define CONN_RETRY_MAX 1000000 //maximum retry interval

namespace CncRemote
{
#define RX_BUFFER_SIZE 1024

	State::State()
	{
		memset(&position, 0, (long)((long)&interpState - (long)&position));
		errorCount = 0;
		messageCount = 0;
		fileCount = 0;
		machineState = mcNO_SERVER;
	}
} //namespace CncRemote

