#include "cncserver.h"
#include <sstream>

//#include "millisleep.h"
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
    pthread_mutex_init(&m_stateLock, NULL);
    pthread_mutex_init(&m_syncLock, NULL);
    pthread_mutex_lock(&m_syncLock);
    m_statePacket.cmd = Comms::cmdNULL;
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
    CActiveSocket * client = m_socket->Accept();
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
        UpdateState();
        pthread_mutex_lock(&m_stateLock);
        m_state.SerializeToString(&m_statePacket.data);
        m_statePacket.cmd = Comms::cmdSTATE;
        pthread_mutex_unlock(&m_stateLock);
    }
    pthread_mutex_unlock(&m_syncLock);
    pthread_mutex_lock(&m_syncLock);
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
    pthread_mutex_lock(&m_stateLock);
    ret = m_statePacket;
    pthread_mutex_unlock(&m_stateLock);
    return ret;
}

Connection::Connection(CActiveSocket * socket, Server * server) : Comms(socket, server)
{
    m_socket = socket;
    m_server = server;
    m_thread = 0;
    m_closing = false;
//    m_socket->SetReceiveTimeout(1,0);
//    m_socket->SetSendTimeout(1,0);
}

Connection::~Connection()
{
    m_server->RemoveConn(this);
}

/*
COMERROR Connection::Connect(const CncString address)
{
    Disconnect();
    int ok;
    m_socket = zmq_socket(m_context, ZMQ_ROUTER);
    if(m_socket == NULL) return errSOCKET;
    ok = zmq_bind(m_socket, to_utf8(address).c_str());
    if(ok < 0)
    {
        zmq_close(m_socket);
        m_socket = NULL;
        return errBIND;
    }
    return errOK;
}*/

/*
COMERROR Connection::SendState(const string& id)
{
    if(!m_socket)
    {
        return errNOSOCKET;
    }
    if(!SendCommand(cmdSTATE, this))
    {
        return errFAILED;
    }
    return errOK;
}*/




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
    SendPacket(pkt);
}

COMERROR Connection::Run()
{
    if(m_thread)
    {
        return errRUNNING;
    }
    SetTimeout(CONN_TIMEOUT);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&m_thread, &attr, Entry, (void *)this);
    if(rc)
    {
        return errNOTHREAD;
    }
    return errOK;
}

void * Connection::Entry(void * param)
{
    Connection * c = (Connection *) param;
    void * ret = c->Entry();
    delete c;
    return(ret);
}

void * Connection::Entry()
{
    while(!m_closing && Poll() != errCONNECT)
    {
    }
    return NULL;
}




} //namespace CncRemote

