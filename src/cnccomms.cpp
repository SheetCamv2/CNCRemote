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




#include "cnccomms.h"
#include <stdio.h>
#include <string.h>

#define CONN_RETRY_START  5000 //first connection retry after 5ms
#define CONN_RETRY_MAX 1000000 //maximum retry interval

namespace CncRemote
{

#ifdef _WIN32
	Mutex::Mutex()
	{
		m_mutex = CreateMutex(NULL, false, NULL);
	}

	Mutex::~Mutex()
	{
		CloseHandle(m_mutex);
	}

	void Mutex::Lock()
	{
		WaitForSingleObject(m_mutex, INFINITE);
	}

	void Mutex:: Unlock()
	{
		ReleaseMutex(m_mutex);
	}
#else
	Mutex::Mutex()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}

	Mutex::~Mutex()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	void Mutex::Lock()
	{
		pthread_mutex_lock(&m_mutex);
	}

	void Mutex:: Unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}
#endif // _WIN32




#define RX_BUFFER_SIZE 1024

	State::State()
	{
		memset(this, 0, sizeof(State));
		errorCount = 0;
		messageCount = 0;
		fileCount = 0;
		machineStatus = mcNO_SERVER;
		m_mutex = NULL;
		m_count = NULL;
	}

	State::State(const State& src, Mutex& mutex)
	{
		*this = src;
		m_mutex = &mutex;
		m_count = new int;
		*m_count = 1;
	}

	State::State(const State& src)
	{
	    *this = src;
	    if(m_count) m_count++;
	}

	State::~State()
	{
		if (m_count)
        {
            (*m_count)--;
            if(*m_count <= 0 && m_mutex)
            {
                m_mutex->Unlock();
                delete m_count;
            }
        }
	}
} //namespace CncRemote

