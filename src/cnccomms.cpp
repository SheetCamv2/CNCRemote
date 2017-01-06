#include "cnccomms.h"
#include <stdio.h>
#include <string.h>

namespace CncRemote{

Comms::Comms()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_socket = NULL;
	m_context = zmq_ctx_new();
}

Comms::~Comms()
{
	zmq_close(m_socket);
    zmq_ctx_destroy(m_context);
}

bool Comms::SendPacket(const Packet &packet)
{
    int rc = zmq_send (m_socket, NULL, 0, ZMQ_SNDMORE | ZMQ_DONTWAIT); //delimiter
	if(rc < 0) return false;

    zmq_msg_t message;
    zmq_msg_init_size (&message, packet.data.size() + 1);
	uint8_t * ptr = (uint8_t *)zmq_msg_data (&message);
	*ptr++ = packet.cmd;
    memcpy (ptr, packet.data.c_str(), packet.data.size());
    rc = zmq_msg_send (&message, m_socket, ZMQ_DONTWAIT);
	if(rc < 0) return false;
	return true;
}

#define DELIMSIZE 10

bool Comms::RecvPacket(Packet &packet)
{
	char buf[DELIMSIZE];
	int ret = zmq_recv (m_socket, buf, DELIMSIZE, ZMQ_DONTWAIT);
	size_t bufSize = DELIMSIZE;
	if(ret < 0 ||
		zmq_getsockopt(m_socket, ZMQ_RCVMORE, buf, &bufSize) < 0 ||
		buf[0] == 0)
	{
		return false;
	}

    zmq_msg_t msg;
    zmq_msg_init (&msg);
	ret = zmq_msg_recv (&msg, m_socket, ZMQ_DONTWAIT);
	if(ret < 0)
	{
		zmq_msg_close (&msg);
		return false;
	}
	bufSize = zmq_msg_size (&msg);
	char * ptr = (char *)zmq_msg_data(&msg);
	bool r = false;
	if(bufSize > 0)
	{
		packet.cmd = *ptr++;
		bufSize--;
		packet.data=string(ptr, bufSize);
		r = true;
	}else
	{
		packet.cmd = cmdNULL;
	}
	zmq_msg_close (&msg);
	return r;
}


int Comms::RecvString(string& data)
{
	zmq_msg_t msg;
	zmq_msg_init (&msg);
	int ret = zmq_msg_recv (&msg, m_socket, ZMQ_NOBLOCK);
	if(ret >= 0)
	{
		string sret((char *)zmq_msg_data (&msg), zmq_msg_size (&msg));
		data = sret;
	}
	zmq_msg_close (&msg);
	return ret;
}

bool Comms::SendCommand(const uint16_t cmd)
{
    if(m_socket < 0) return false;
	Packet packet;
	packet.cmd = cmd;
	return SendPacket(packet);
}

bool Comms::SendCommand(const uint16_t cmd, const bool state)
{
	CmdBuf buf;
	buf.set_state(state);
	return SendCommand(cmd, buf);
}

bool Comms::SendCommand(const uint16_t cmd, const double value)
{
	CmdBuf buf;
	buf.set_rate(value);
	return SendCommand(cmd, buf);
}

bool Comms::SendCommand(const uint16_t cmd, const string value)
{
	CmdBuf buf;
	buf.set_string(value);
	return SendCommand(cmd, buf);
}


bool Comms::SendCommand(const uint16_t command, const CmdBuf& data)
{
    if(m_socket < 0) return false;
	Packet packet;
    data.SerializeToString(&packet.data);
	packet.cmd = command;
	return SendPacket(packet);
}

} //namespace CncRemote

