#include "cncserver.h"

namespace CncRemote
{

Server::Server()
{
}

Server::~Server()
{
}

Comms::COMERROR Server::Connect(const string address)
{
    Disconnect();
    int ok;
    m_socket = zmq_socket(m_context, ZMQ_ROUTER);
    if(m_socket == NULL) return errSOCKET;
    ok = zmq_bind(m_socket, address.c_str());
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

    COMERROR ret = errOK;
    bool sent = false;
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


bool Server::Poll()
{
    if(m_socket == NULL) return false;
    bool ret = false;
	char *buf = NULL;
	bool needState = false;
	while(1)
	{
		string ident;
        int ret = RecvString(ident);
        if(ret < 0)
        {
            break;
        }
/*        int id = -1;
        for(unsigned int ct=0; ct < m_ids.size(); ct++)
        {
            if(m_ids[ct].name == ident)
            {
                id = ct;
                break;
            }
        }
        if(id <0)
        {
            Identity i;
            i.name = "wed";//ident;
            i.timeout = time(NULL) + CONN_TIMEOUT;
            m_ids.push_back(i);
        }else
        {
            m_ids[id].timeout = time(NULL) + CONN_TIMEOUT;
        }*/
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
			HandlePacket(pkt);
		}
        ret = true;
    }
    if(needState)
    {
        UpdateState();
        SendState(m_curId);
    }
/*    time_t t = time(NULL);
    for(int ct=0; ct < m_ids.size(); ct++)
    {
        Identity & id = m_ids[ct];
        if(t > id.timeout && false)
        {
printf("ID %s timed out\n", id.name.c_str());
            m_ids.erase(m_ids.begin() + ct);
            ct--;
        }
    }*/
	return ret;
}


} //namespace CncRemote

