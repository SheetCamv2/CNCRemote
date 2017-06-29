#include "cnccomms.h"
#include <stdio.h>
#include <string.h>

#define CONN_RETRY_START  5000 //first connection retry after 5ms
#define CONN_RETRY_MAX 1000000 //maximum retry interval

namespace CncRemote
{


Comms::Comms(CActiveSocket *socket, Server * server)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_socket = socket;
    m_socket->DisableNagleAlgoritm();
    m_connState = (CONNSTATE)-1;
}

Comms::Comms()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_socket = NULL;
    m_connState = (CONNSTATE)-1;
    m_connTime = CONN_RETRY_START;
}

Comms::~Comms()
{
    if(m_socket)
    {
        delete m_socket;
    }
}
#define RX_BUFFER_SIZE 1024

void Comms::Connect(const CncString& address, const uint32_t port)
{
    m_rxData.clear();
    m_address = address;
    m_port = port;
    if(!m_socket) return;
    m_socket->Close();
    Connected(connNONE);
}


COMERROR Comms::Poll()
{
    if(!m_socket) return errNOSOCKET;
    if(m_connState == connNONE && !m_server && !m_address.empty()) //try to auto reconnect
    {
        if(m_connTimer.GetElapsed() < m_connTime) return errCONNECT;
        m_connTimer.Restart();
        m_connTime *= 2; //increase retry time
        if(m_connTime > CONN_RETRY_MAX)
        {
            m_connTime = CONN_RETRY_MAX;
        }
        m_socket->Close();
        m_socket->Initialize();
        SetTimeout(m_socketTimeout);
        bool ok = m_socket->Open(m_address.c_str(), m_port);
        if(!ok)
        {
            Connected(connNONE);
            return errCONNECT;
        }
        m_socket->DisableNagleAlgoritm();
        m_connTimer.Restart();
        Connected(connNETWORK);
    }
    uint8_t buf[RX_BUFFER_SIZE];
    int bytes = m_socket->Receive(RX_BUFFER_SIZE, buf);
    if(bytes <= 0) // error
    {
        CSimpleSocket::CSocketError e = m_socket->GetSocketError();
        if(e== CSimpleSocket::SocketEwouldblock ||
           e == CSimpleSocket::SocketInterrupted)
        {
            if(m_connTimer.GetElapsed() > CONN_TIMEOUT)
            {
                Connected(connNETWORK);
            }
            return errNODATA;
        }
        m_rxData.clear();
        Connected(connNONE);
        return errCONNECT;
    }
    m_connTimer.Restart();
    Connected(connDATA);
    uint8_t * ptr = buf;
    for(int ct = bytes; ct >0; ct--)
    {
        char c = (char)(*ptr++);
        if(c == 0)
        {
            CobsDecode((const uint8_t *)m_rxData.data(), m_rxData.size());
            m_rxData.clear();
            continue;
        }
        m_rxData += c;
    }
    return errOK;
}

void Comms::SetTimeout(float seconds)
{
    m_socketTimeout = seconds;
    if(!m_socket) return;
    if(seconds > 0.000001)
    {
        m_socket->SetBlocking();
        int32_t s = (int)seconds;
        seconds -= s;
        int32_t us = seconds * 1000000;
        m_socket->SetReceiveTimeout(s,us);
        m_socket->SetSendTimeout(s,us);
        m_socket->SetConnectTimeout(s,us);
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
        m_socket->Initialize();
    }
    m_connTime = CONN_RETRY_START;
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

size_t Comms::CobsEncode(const uint8_t *ptr, size_t length, uint8_t *dst)
{

    const uint8_t * dsts = dst;
    uint8_t *code_ptr = dst++;
    uint8_t code = 0x01;
    const uint8_t *end = ptr + length;
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

    return((dst - dsts) - 1);
}

void Comms::CobsDecode(const uint8_t *ptr, size_t length)
{
    m_packet.data.clear();
    const uint8_t *end = ptr + length;
    while (ptr < end)
    {
        int code = *ptr++;
        for (int i = 1; i < code; i++)
        {
            if(ptr >= end) //malformed data
            {
                OnPacketError();
                return;
            }
            else
            {
                m_packet.data.push_back(*ptr++);
            }
        }
        if (code < 0xFF && ptr != end)
            m_packet.data.push_back((char)0);
    }
    memcpy(&m_packet.cmd, m_packet.data.data(), sizeof(m_packet.cmd));
    m_packet.data.erase(0, sizeof(m_packet.cmd));
    HandlePacket(m_packet);
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
    if(!m_socket || m_connState == connNONE) return false;
/*
for(int g = 0; g < 100000000; g++)
{
string s;
int r = rand() % 2000;
for(int c = 0; c < r; c++)
{
    s += (char)rand();
}
int si = s.size() + (s.size() / 254) + 1;
uint8_t sd[si + 10];
memset(sd,1,si + 10);
int siz = CobsEncode((const uint8_t *)s.data(), s.size(), sd);
sd[siz] = 0;
m_packet.data.clear();
CobsDecode(sd, siz);
if(m_packet.data != s)
{
    int a=m_packet.data.size();
    int b=s.size();
    for(int ct=0; ct < a; ct++)
    {
        if(m_packet.data[ct] != s[ct])
        {
            int g=1;
        }
    }
}
}
*/


    string s;
    s.append((char *)&packet.cmd, sizeof(packet.cmd));
    s += packet.data;
    int size = s.size() + (s.size() / 254) + 2; //maximum data size after encoding plus zero endo of packet
    uint8_t d[size];
    size = CobsEncode((const uint8_t *)s.data(), s.size(), d);
    d[size++] = 0;
    int sent = m_socket->Send(d, size);
    if(sent <=0 || sent != size)
    {
        if(m_socket->GetSocketError() == CSimpleSocket::SocketInvalidSocket)
        {
            Connected(connNONE);
        }
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

void Comms::Connected(const CONNSTATE state)
{
    if(m_connState != state)
    {
        if(m_connState == connDATA) //was connected so short retry interval
        {
            m_connTime = CONN_RETRY_START;
        }
        m_connState = state;
        OnConnection(state);
    }
}


} //namespace CncRemote

