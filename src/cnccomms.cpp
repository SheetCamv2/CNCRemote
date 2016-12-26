#include "cnccomms.h"

#include "zmq.hpp"


CncComms::CncComms(const string ipAddress, const string statePort, const string cmdPort, const bool server)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_context = new zmq::context_t(1);
    m_stateSocket = NULL;

    m_isServer = server;
    string stateAddr = "tcp://" + ipAddress + ":" + statePort;
    string cmdAddr = "tcp://" + ipAddress + ":" + cmdPort;

    if(server)
    {
        m_stateSocket = new zmq::socket_t(*m_context, ZMQ_PUB);
        m_stateSocket->bind(stateAddr.c_str());
        m_cmdSocket = new zmq::socket_t(*m_context, ZMQ_PULL);
        m_cmdSocket->bind(cmdAddr.c_str());
    }else
    {
        m_stateSocket = new zmq::socket_t(*m_context, ZMQ_SUB);
        m_stateSocket->connect (stateAddr.c_str());
        m_stateSocket->setsockopt(ZMQ_SUBSCRIBE,"",0);
        m_cmdSocket = new zmq::socket_t(*m_context, ZMQ_PUSH);
        m_cmdSocket->connect (cmdAddr.c_str());
    }
}

CncComms::~CncComms()
{
    if (m_stateSocket)
    {
        delete m_stateSocket;
    }
    delete m_context;
}


void CncComms::Send()
{
    if(!m_stateSocket)
    {
        CommsException exc("Not initialized");
        return;
    }
    string buf;
    SerializeToString(&buf);
    zmq::message_t request (buf.size());
    memcpy ((void *) request.data (), buf.c_str(), buf.size());
    if(m_stateSocket->send (request, ZMQ_NOBLOCK))
    {
        Clear(); //only clear current data if packet was successfully sent
    }
}


bool CncComms::Poll()
{
    bool ret = false;
    if(m_isServer)
    {
        if(!m_cmdSocket) return false;
        zmq::message_t request;
        while(m_cmdSocket->recv (&request, ZMQ_NOBLOCK))
        {
            string buf(static_cast<char*>(request.data()), request.size());
            CncCmdBuf cmd;
            cmd.ParseFromString(buf);
            ExecuteCommand(cmd);
            ret = true;
        }
        return ret;
    }

    if(!m_stateSocket) return false;
    zmq::message_t request;
    while(m_stateSocket->recv (&request, ZMQ_NOBLOCK))
    {
        std::string buf(static_cast<char*>(request.data()), request.size());
        CncStateBuf state;
        state.ParseFromString(buf);
        MergeFrom(state);
        ret = true;
    }
    return true;
}

void CncComms::SendCommand(const CncCmdBuf& cmd)
{
    if(!m_cmdSocket)
    {
        CommsException exc("Not initialized");
    }
    string buf;
    cmd.SerializeToString(&buf);
    zmq::message_t cmdMsg (buf.size());
    memcpy ((void *) cmdMsg.data (), buf.c_str(), buf.size());
    if(m_cmdSocket->send (cmdMsg))
    {
        CommsException exc("Failed to send data");
    }
}




CncClient::CncClient(const string ipAddress, const string statePort, const string cmdPort) : CncComms(ipAddress, statePort, cmdPort, false)
{

}


void CncClient::DrivesOn(const bool state)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdDRIVESON);
    cmd.set_state(state);
    SendCommand(cmd);
}

void CncClient::JogVel(const AXES velocities)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdJOGVEL);
    CncAxes& axes = *cmd.mutable_axes();
    axes.set_x(velocities.x);
    axes.set_y(velocities.y);
    axes.set_z(velocities.z);
    axes.set_a(velocities.a);
    axes.set_b(velocities.b);
    axes.set_c(velocities.c);
    SendCommand(cmd);
}

void CncClient::Mdi(const string line)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdMDI);
    cmd.set_string(line);
    SendCommand(cmd);
}


void CncClient::SetFRO(const double percent)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdFRO);
    cmd.set_rate(percent);
    SendCommand(cmd);
}

void CncClient::LoadFile(const string file)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdFILE);
    cmd.set_string(file);
    SendCommand(cmd);
}

void CncClient::CycleStart()
{
    CncCmdBuf cmd;
    cmd.set_type(cmdSTART);
    SendCommand(cmd);
}

void CncClient::Stop()
{
    CncCmdBuf cmd;
    cmd.set_type(cmdSTOP);
    SendCommand(cmd);
}

void CncClient::Pause(const bool state)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdPAUSE);
    cmd.set_state(state);
    SendCommand(cmd);
}


void CncClient::BlockDelete(const bool state)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdBLOCKDEL);
    cmd.set_state(state);
    SendCommand(cmd);
}

void CncClient::SingleStep(const bool state)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdSINGLESTEP);
    cmd.set_state(state);
    SendCommand(cmd);
}

void CncClient::OptionalStop(const bool state)
{
    CncCmdBuf cmd;
    cmd.set_type(cmdOPTSTOP);
    cmd.set_state(state);
    SendCommand(cmd);
}
