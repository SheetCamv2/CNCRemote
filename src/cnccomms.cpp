#include "cnccomms.h"
#include <stdio.h>
#include <string.h>

namespace CncRemote
{


Comms::Comms(CActiveSocket *socket, Server * server)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_socket = socket;
}

Comms::~Comms()
{
    Close();
}
#define RX_BUFFER_SIZE 1024

COMERROR Comms::Poll()
{
    uint8_t buf[RX_BUFFER_SIZE];
    int bytes = m_socket->Receive(RX_BUFFER_SIZE, buf);
    if(bytes <= 0) // error
    {
        if(m_socket->GetSocketError() == CSimpleSocket::SocketEwouldblock)
        {
            return errNODATA;
        }
        return errCONNECT;
    }
    CobsDecode(buf,bytes);
    return errOK;
}

void Comms::SetTimeout(float seconds)
{
    if(!m_socket) return;
    if(seconds > 0.000001)
    {
        m_socket->SetBlocking();
        int32_t s = (int)seconds;
        seconds -= s;
        int32_t us = seconds * 1000000;
        m_socket->SetReceiveTimeout(s,us);
        m_socket->SetSendTimeout(s,us);
    }
    else
    {
        m_socket->SetNonblocking();
    }
}

void Comms::Close()
{
    if(m_socket)
    {
        m_socket->Close();
    }
}

/*
COMERROR Comms::Run()
{
    if(m_thread)
    {
        return errRRUNNING;
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

static void * Comms::Entry(void * t)
{
    return (Comms *)t->Entry();
}

void * Comms::Entry()
{
    void * ret = NULL;
    while(!m_closing)
    {
        if(Poll() != errOK)
        {
            return ((void *)1);
        }
    }
    return NULL;
}

bool Comms::Poll()
{
    if(!m_socket) return;

#ifdef USE_PLUGINS
    if(m_plugin) m_plugin->Poll();
#endif
    bool ret = false;
    while(1)
    {

        Packet pkt;
        if(!RecvPacket(pkt))
        {
            break;
        }
        switch(pkt.cmd)
        {
        case cmdNULL:
            break;

        case cmdSENDFILE: //TODO: File handling
            break;

        case cmdREQFILE: //TODO: File handling
            break;

        case cmdPING:
            m_pingResp = true;
            break;

        default:
            HandlePacket(pkt);
        }
        ret = true;
    }
    if(ret)
    {
        m_timeout = time(NULL) + CONN_TIMEOUT;
    }

    m_isConnected = (m_timeout > time(NULL));
    if(m_isConnected)
    {
        SendCommand(cmdSTATE);
    }
    if(m_isConnected != m_wasConnected)
    {
        m_wasConnected = m_isConnected;
        if(!m_isConnected)
        {
printf("Connection timed out\n");
            SendCommand(cmdSTATE); //queue a state message to wake up the server when we connect
        }
else
printf("Regained connection\n");
        OnConnection(m_isConnected);
    }
    return ret;
}
*/


#define FinishBlock(X) (*code_ptr = (X), code_ptr = dst++, code = 0x01)

void Comms::CobsEncode(const uint8_t *ptr, size_t length, uint8_t *dst)
{
    uint8_t *code_ptr = dst++;
    uint8_t code = 0x01;
    while(length > 0)
    {
        const uint8_t *end = ptr;
        if(length > 254)
        {
            length -= 254;
            end += 254;
        }
        else
        {
            end += length;
            length = 0;
        }
        while (ptr < end)
        {
            if (*ptr == 0)
                FinishBlock(code);
            else
            {
                *dst++ = *ptr;
                if (++code == 0xFF)
                    FinishBlock(code);
            }
            ptr++;
        }
        FinishBlock(code);
    }
}

void Comms::CobsDecode(const uint8_t *ptr, size_t length)
{
    const uint8_t *end = ptr + length;
    while (ptr < end)
    {
        int code = *ptr++;
        while(code == 0 && ptr < end) //end of packet
        {
            if(m_packet.data.size() < sizeof(m_packet.cmd))
            {
               OnPacketError();
            }
            memcpy(&m_packet.cmd, &m_packet.data[0], sizeof(m_packet.cmd));
            m_packet.data.erase(0, sizeof(m_packet.cmd));
            HandlePacket(m_packet);
            m_packet.data.clear();
            code = *ptr++;
        }
        for (int i = 1; i < code; i++)
        {
            if(*ptr == 0 //Unexpected end of packet marker.
               || ptr >= end) //malformed data
            {
                OnPacketError();
                code = 0xFF;
                break;
            }
            else
            {
                m_packet.data.push_back(*ptr++);
            }
        }
        if (code < 0xFF)
            m_packet.data.push_back(0);
    }
}


/*
void Comms::ProcessByte(const uint8_t byte)
{
    if(byte == pktESCAPE)
    {
        if(!m_wasEscape)
        {
            m_wasEscape = true;
            return;
        }
    }
    if(m_wasEscape) //start of packet or escape char
    {
        m_wasEscape = false;
        if(byte != pktESCAPE) //start of packet
        {
            if(m_rxCount != 0)
            {
                printf("Last packet was invalid\n"));
            }
            m_rxCount = 0;
            m_header.bytes = 0;
            m_packet.data.clear();
        }
    }

    if(m_rxCount < sizeof(m_header))
    {
        char * ptr = (char *)&m_header;
        ptr[m_rxCount] = byte;
        m_rxCount++;
        return;
    }
    if(m_rxCount >= MAX_PACKET_SIZE)
    {
        return;
    }
    m_rxCount++;
    m_packet.data.push_back(byte);
    if(m_rxCount == m_header.bytes)
    {
        m_packet.cmd = m_header.cmd;

    }
}
*/

bool Comms::SendPacket(const Packet &packet)
{
    if(!m_socket) return false;

    string s;
    s.append((char *)&packet.cmd, sizeof(packet.cmd));
    s += packet.data;
    int size = s.size() + (s.size() / 254) + 2;
    uint8_t d[size];
    CobsEncode((const uint8_t *)s.data(), s.size(), d);
    d[size-1] = 0;
    int sent = m_socket->Send(d, size);
    if(sent <=0 || sent != size)
    {
        return false;
    }
    return true;
}

/*
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
    }
    else
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
}*/

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

