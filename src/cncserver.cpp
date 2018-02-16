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


#include "cncserver.h"
#include <sstream>

#include "timer.h"

namespace CncRemote
{

Server::Server()
{
    m_timeout = CONN_TIMEOUT;
    m_socket = new CPassiveSocket(CSimpleSocket::SocketTypeTcp);
    m_socket->Initialize();
    SetTimeout(0);
    m_listening = false;
    MUTEX_CREATE(m_stateLock);
    MUTEX_CREATE(m_syncLock);
    MUTEX_LOCK(m_syncLock);
    m_statePacket.hdr.cmd = Comms::cmdNULL;
    m_state.set_gcode_units(1);
}

Server::~Server()
{
    delete m_socket;
}

void Server::SetTimeout(const float seconds)
{
   m_timeout = seconds;
   SetTimeout();
}

void Server::SetTimeout()
{
    if(m_timeout > 0.000001)
    {
        float seconds = m_timeout;
        m_socket->SetBlocking();
        int32_t s = (int)seconds;
        seconds -= s;
        int32_t us = seconds * 1000000;
        m_socket->SetReceiveTimeout(s,us);
        m_socket->SetSendTimeout(s,us);
        m_socket->SetConnectTimeout(s, us);
    }else
    {
        m_socket->SetNonblocking();
    }
}

COMERROR Server::Bind(const uint32_t port)
{
    if(!m_socket->Listen(NULL, port))
    {
        return errBIND;
    }
    return errOK;
}

COMERROR Server::Poll()
{
    MUTEX_UNLOCK(m_syncLock);
    CActiveSocket * client = m_socket->Accept();
    MUTEX_LOCK(m_syncLock);
    if(client)
    {
        printf("Accepting connection from %s\n", client->GetClientAddr());
        Connection * conn = CreateConnection(client, this);
        if(conn->Run() != errOK)
        {
            printf("Failed to create thread\n");
            delete conn;
            return errTHREAD;
        }
        m_conns.push_back(conn);
    }
    if(m_conns.size() > 0)
    {
        MUTEX_LOCK(m_stateLock);
        UpdateState();
        m_state.SerializeToString(&m_statePacket.data);
        m_statePacket.hdr.cmd = Comms::cmdSTATE;
        MUTEX_UNLOCK(m_stateLock);
    }
    return errOK;
}


void Server::RemoveConn(Connection * conn)
{
    for(size_t ct=0; ct < m_conns.size(); ct++)
    {
        if(m_conns[ct] == conn)
        {
            m_conns.erase(m_conns.begin() + ct);
            return;
        }
    }
}

Packet Server::GetState()
{
    Packet ret;
    MUTEX_LOCK(m_stateLock);
    ret = m_statePacket;
    MUTEX_UNLOCK(m_stateLock);
    return ret;
}


Connection::Connection(CActiveSocket * socket, Server * server) : Comms(socket, server)
{
    m_socket = socket;
    m_server = server;
    m_thread = 0;
    m_closing = false;
	m_loadFile = NULL;
	m_loadLength = 0;
	m_loadCount = 0;
}

Connection::~Connection()
{
    RemoveTemp();
	printf("%s disconnected\n", m_socket->GetClientAddr());
	m_server->RemoveConn(this);
}

#include "timer.h"
TestTimer ttm("packet");
COMERROR Connection::Poll()
{
    COMERROR ret = Comms::Poll();
    if(ret != errOK) return ret;
    UpdateState();
	return ret;
}

void Connection::UpdateState()
{
    Packet pkt = m_server->GetState();
	pkt.hdr.heartBeat = m_packet.hdr.heartBeat;
    SendPacket(pkt);
}

COMERROR Connection::Run()
{
    if(m_thread)
    {
        return errRUNNING;
    }
    SetTimeout(CONN_TIMEOUT / 2000000); //Half of CONN_TIMEOUT time. CONN_TIMEOUT is in microseconds.

#ifdef _WIN32
    m_thread = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size
        Entry,                  // thread function name
        this,                   // argument to thread function
        0,                      // use default creation flags
        NULL);                  // returns the thread identifier

    if(m_thread == NULL)
    {
        return errNOTHREAD;
    }
#else
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&m_thread, &attr, Entry, (void *)this);
    if(rc)
    {
        return errNOTHREAD;
    }
#endif // _WIN32
    return errOK;
}

#ifdef _WIN32
DWORD WINAPI Connection::Entry( LPVOID param )
#else
void * Connection::Entry(void * param)
#endif // _WIN32
{
    Connection * c = (Connection *) param;
    c->Entry();
    delete c;
    return(NULL);
}

void Connection::Entry()
{
    SetTimeout(0.005);
	while(!m_closing && Poll() != errCONNECT)
    {
    }
}

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


} //namespace CncRemote

