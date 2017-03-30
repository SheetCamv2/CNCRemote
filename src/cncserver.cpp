#include "cncserver.h"
#include <sstream>

namespace CncRemote
{

Server::Server()
{
}

Server::~Server()
{
}

CncString Server::GenerateTcpAddress(const int port)
{
#ifdef _USING_WINDOWS
	wstringstream stream;
#define _TT(n) L##n
#else
	stringstream stream;
#define _TT(n) n
#endif
	stream <<  _TT("tcp://*:") << port;
	return stream.str();
}

Comms::COMERROR Server::Connect(const CncString address)
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
}

void Server::Disconnect()
{

}



Comms::COMERROR Server::SendState(const string& id)
{
    if(!m_socket)
    {
        return errNOSOCKET;
    }
/*    if(m_ids.empty()) //nothing to do
    {
        return(errOK);
    }*/

    string s;
    SerializeToString(&s);
    zmq_msg_t payload;
    zmq_msg_init_size (&payload, s.size() + 1);
	uint8_t * ptr = (uint8_t *)zmq_msg_data (&payload);
	*ptr++ = cmdSTATE;
    memcpy (ptr, s.c_str(), s.size());

    int rc = zmq_send (m_socket,id.data(), id.size(), ZMQ_SNDMORE | ZMQ_DONTWAIT);
    if(rc < 0)
    {
        return errFAILED;
    }
    rc = zmq_send (m_socket, NULL, 0, ZMQ_SNDMORE | ZMQ_DONTWAIT); //delimiter
    if(rc < 0)
    {
        return errFAILED;
    }

    rc = zmq_msg_send (&payload, m_socket, ZMQ_DONTWAIT);
    if(rc < 0)
    {
        return errFAILED;
    }
    return errOK;
}

#include "timer.h"
TestTimer ttm("packet");
bool Server::Poll()
{
    if(m_socket == NULL) return false;
    bool ret = false;
	bool needState = false;
	while(1)
	{
		string ident;
        int ret = RecvString(ident);
        if(ret < 0)
        {
            break;
        }
        m_curId = ident;

		Packet pkt;
		if(!RecvPacket(pkt))
		{
			break;
		}
//        m_ids[id].needState = true;
        needState = true;
		switch(pkt.cmd)
		{
		case cmdNULL:
			break;

        case cmdSTATE:
            break;

		case cmdSENDFILE: //TODO: File handling
			break;

		case cmdREQFILE: //TODO: File handling
			break;

		default:
		{
		    ttm.Restart();
			HandlePacket(pkt);
			ttm.Check();

		}
		}
        ret = true;
    }
    if(needState)
    {
        UpdateState();
        SendState(m_curId);
    }
	return ret;
}


} //namespace CncRemote

