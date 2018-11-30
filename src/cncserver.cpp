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

	enum { MAX_THREADS = 8 };


Server::Server()
{
	m_server = NULL;
	m_file = NULL;
}

Server::~Server()
{
	DeleteTemp();
	delete m_server;
}


#define BIND0(name) m_server->bind(#name, [this]() {return name();});
#define BIND1(name, type1) m_server->bind(#name, [this](type1 param1) {return name(param1);});
#define BIND2(name, type1, type2) m_server->bind(#name, [this](type1 param1, type2 param2) {return name(param1, param2);});

COMERROR Server::Bind(const uint32_t port)
{
	delete m_server;
	m_server = new rpc::server(port);
	m_server->suppress_exceptions(true);

	m_server->bind("GetState", [this]() {return GetState2();});
	BIND1(DrivesOn, bool);
	BIND1(JogVel, Axes);
	BIND2(JogStep, Axes, double);
	BIND1(Mdi, string);
	BIND1(FeedOverride, double);
	BIND1(SpindleOverride, double);
	BIND1(RapidOverride, double);
	m_server->bind("LoadFile", [this](string param1) {return LoadFile2(param1);});
	BIND0(CloseFile);
	BIND0(CycleStart);
	BIND0(CycleStop);
	BIND1(FeedHold, bool);
	BIND1(BlockDelete, bool);
	BIND1(SingleStep, bool);
	BIND1(OptionalStop, bool);
	BIND1(Home, BoolAxes);
	BIND1(GetOffset, unsigned int);
	BIND1(SendInit, string);
	BIND2(SendData, string, int);
	BIND1(GetError, int);
	BIND1(GetMessage, int);

	m_server->bind("Ping", []() {return true; });
	m_server->bind("Version", []() {return (CNCREMOTE_PROTOCOL_VERSION); });

	m_server->async_run(MAX_THREADS);
	return errOK;
}

COMERROR Server::Poll()
{
	if (!m_server) return errCONNECT;
    return errOK;
}

StatePtr Server::GetState()
{
	StatePtr s(&m_state, &m_syncLock);
	return s;
}

string Server::SendInit(string nameHint)
{
	CloseFile();
	DeleteTemp();
	m_curBlock = 0;
#ifdef _WIN32
	char szTempFileName[MAX_PATH];
	char lpTempPathBuffer[MAX_PATH];
	DWORD dwRetVal = GetTempPathA(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		return("");
	}

	//  Generates a temporary file name.
	UINT uRetVal = GetTempFileNameA(lpTempPathBuffer, nameHint.c_str(), 0, szTempFileName);
	if (uRetVal == 0)
	{
		return("");
	}
	m_file = fopen(szTempFileName, "w");
	if (m_file)
	{
		m_curFile = szTempFileName;
		return szTempFileName;
	}
	return("");
#else
	char path[PATH_MAX];
	sprintf(path,"/tmp/CncRemote%sXXXXXX", nameHint.c_str());
	int fd = mkstemp(path);
	if (fd < 0) return ("");
	m_file = fdopen(fd, "w");
	if (!m_file) return ("");
	m_curFile = path;
	return path;
#endif

}

bool Server::SendData(const string data, const int block)
{
	if (!m_file) return false;
	if (block != m_curBlock)
	{
		fclose(m_file);
		return false;
	}
	m_curBlock = block + 1;
	int l = data.size();
	int r = fwrite(data.c_str(), 1, l, m_file);
	if (r != l)
	{
		fclose(m_file);
		return false;
	}
	if (l < FILE_BLOCK_SIZE) fclose(m_file);
	return true;
}


void Server::DeleteTemp()
{
	if (m_file) fclose(m_file);
	if (m_curFile.empty()) return;
	remove(m_curFile.c_str());
}

State Server::GetState2()
{
	{
		Sync();
		UpdateState(m_state);
	}
	return State(m_state, m_syncLock);
}

bool Server::LoadFile2(const string file)
{
	m_state.fileCount++;
	return LoadFile(file);
}

string Server::GetError(const unsigned int index)
{
	if (index >= m_errors.size()) return string();
	return m_errors[index];
}

string Server::GetMessage(const unsigned int index)
{
	if (index >= m_messages.size()) return string();
	return m_messages[index];
}

void Server::LogError(string error)
{
	if (error.size() > 0)
	{
		m_errors.push_back(error);
		m_state.errorCount = m_errors.size();
	}
}

void Server::LogMessage(string message)
{
	if (message.size() > 0)
	{
		m_messages.push_back(message);
		m_state.messageCount = m_messages.size();
	}
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

