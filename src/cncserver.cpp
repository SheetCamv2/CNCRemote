/****************************************************************
CNCRemote server
Copyright 2017-2018 Stable Design <les@sheetcam.com>


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


#include "cncserver.h"
#include <sstream>

#include "timer.h"

namespace CncRemote
{

#define MAX_THREADS 8

Server::Server()
{
    MUTEX_CREATE(m_syncLock);
    MUTEX_LOCK(m_syncLock);
	m_server = NULL;
}

Server::~Server()
{
	delete m_server;
	MUTEX_DESTROY(m_syncLock);
}

#define BIND(name, type) m_server->bind(#name, [this](type param) {return name(param);});

#define BINDVOID(name) m_server->bind(#name, [this]() {return name();});

COMERROR Server::Bind(const uint32_t port)
{
	delete m_server;
	m_server = new rpc::server(port);
	m_server->suppress_exceptions(true);
	BINDVOID(GetState);
	BIND(DrivesOn, bool);
	BIND(JogVel, Axes);
	BIND(Mdi, string);
	BIND(FeedOverride, double);
	BIND(SpindleOverride, double);
	BIND(RapidOverride, double);
	BIND(LoadFile, string);
	BINDVOID(CloseFile);
	BINDVOID(CycleStart);
	BINDVOID(CycleStop);
	BIND(FeedHold, bool);
	BIND(BlockDelete, bool);
	BIND(SingleStep, bool);
	BIND(OptionalStop, bool);
	BIND(Home, BoolAxes);
	m_server->bind("Ping", []() {return true; });
	m_server->bind("Version", []() {return PROTOCOL_VERSION; });

	m_server->async_run(MAX_THREADS);
	return errOK;
}

COMERROR Server::Poll()
{
	if (!m_server) return errCONNECT;
    MUTEX_UNLOCK(m_syncLock);
    MUTEX_LOCK(m_syncLock);
    return errOK;
}

/*
void Connection::RemoveTemp()
{
	if(m_loadFile) fclose(m_loadFile);
	m_loadFile = NULL;
	if(m_tempFileName.size() > 0)
	{
#ifdef _WIN32
		_wremove(m_tempFileName.c_str());
#else	
		remove(m_tempFileName.c_str());
#endif
		m_tempFileName.clear();
	}
}

void Connection::RecieveFileInit(const CmdBuf& cmd)
{
	RemoveTemp();
#ifdef _WIN32
	wchar_t buf[MAX_PATH];
	if(GetEnvironmentVariableW(_T("TEMP"), buf, MAX_PATH) == 0)
	{
		printf("Failed to find temp directory");
		return;
	}
	CncString path = buf;
	path += _T("\\CNCRemote");
	if(_wmkdir(path.c_str()) < 0 && errno != EEXIST)
	{
		printf("Failed to make temp directory");
		return;
	}
	path += _T('\\');
	path += from_utf8(cmd.string().c_str());
	m_loadFile = _wfopen(path.c_str(), _T("wb"));
	if(!m_loadFile)
	{
		printf("Failed to create temp file");
		return;
	}
	m_tempFileName = path;
	m_loadLength = cmd.intval();
	m_loadCount = 0;
#else
#endif
}

void Connection::RecieveFileData(const CmdBuf& cmd)
{
	if(!m_loadFile)
	{
		printf("Unexpected file data");
		return;
	}
	int len = cmd.string().size();
	fwrite(cmd.string().data(), 1, len, m_loadFile);
	m_loadCount += len;
	if(m_loadCount < m_loadLength)
	{
		return;
	}
	fclose(m_loadFile);
	m_loadFile = NULL;
	if(m_loadCount != m_loadLength)
	{
		printf("Received file is wrong length");
	}
}
*/

} //namespace CncRemote

