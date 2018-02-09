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
    if(m_connState == connNONE && !m_address.empty()) //try to auto reconnect if client
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
        bool ok = m_socket->Open(to_utf8(m_address).c_str(), m_port);
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

bool Comms::SendPacket(const Packet &packet)
{
    if(!m_socket || m_connState == connNONE) return false;

    string s;
    s.append((char *)&packet.cmd, sizeof(packet.cmd));
    s += packet.data;
    int size = s.size() + (s.size() / 254) + 2; //maximum data size after encoding plus zero endo of packet
    uint8_t * d = new uint8_t[size];
    size = CobsEncode((const uint8_t *)s.data(), s.size(), d);
    d[size++] = 0;
    int sent = m_socket->Send(d, size);
	delete d;
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

bool Comms::SendCommand(const uint16_t cmd)
{
    if(m_socket == NULL) return false;
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

