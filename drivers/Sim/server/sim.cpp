/****************************************************************
CNCRemote simulator
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

#include "sim.h"


struct MACHINESTATE
{
    bool controlOn;
    bool paused;
    bool blockDelete;
    bool optStop;
    bool running;
    bool step;
    double fro;
    int curLine;
	int busy;
} g_machineState;

CncRemote::Axes g_jogVel;
CncRemote::Axes g_curPos;

const char *g_cmdNames[]={
    "cmdNULL",
    "cmdPING",
    "cmdSTATE",
    "cmdDRIVESON",
    "cmdJOGVEL",
    "cmdJOGSTEP",
    "cmdMDI",
    "cmdFRO",
    "cmdFILE",
    "cmdCLOSEFILE",
    "cmdSTART",
    "cmdSTOP",
    "cmdPAUSE",
    "cmdBLOCKDEL",
    "cmdSINGLESTEP",
    "cmdOPTSTOP",
    "cmdSENDFILEINIT",
    "cmdSENDFILEDATA",
	"cmdCOOLANT",
    "cmdMIST",
    "cmdSPINDLE",
    "cmdHOME",
};

class SimConnection : public Connection
{
public:
    SimConnection(CActiveSocket * client, Server * server) : Connection(client, server)
    {
        SetTimeout(1);
    }

    virtual ~SimConnection()
    {

    }

    virtual void OnConnection()
    {

    }

    virtual void HandlePacket(const Packet& pkt)
    {
        CncRemote::CmdBuf cmd;
        if(pkt.data.size() > 0 &&
            !cmd.ParseFromString(pkt.data))
        {
            OnPacketError();
            return;
        }
        if(pkt.hdr.cmd < cmdMAX && pkt.hdr.cmd != cmdSTATE)
        {
            printf("Command %d %s\n", pkt.hdr.cmd, g_cmdNames[pkt.hdr.cmd]);
        }
        switch(pkt.hdr.cmd)
        {
        case cmdDRIVES_ON:
            g_machineState.controlOn = cmd.state();
            break;

        case cmdJOG_VEL:
            g_jogVel.CopyFrom(cmd.axes());
            break;

        case cmdMDI:
			g_machineState.busy = 1000;
            printf("MDI:%s\n", cmd.string().c_str());
            break;

        case cmdFRO:
            g_machineState.fro = cmd.rate();
            break;

        case cmdFILE:
            break;

        case cmdCLOSE_FILE:
            break;

        case cmdSTART:
            if(g_machineState.paused)
            {
                g_machineState.paused = false;
                break;
            }
            g_machineState.running = true;
            break;

        case cmdSTOP:
            g_machineState.running = false;
            break;

        case cmdFEED_HOLD:
            g_machineState.paused = true;
            break;

        case cmdBLOCK_DEL:
            g_machineState.blockDelete = cmd.state();
            break;

        case cmdSINGLE_STEP:
            g_machineState.step = cmd.state();
            break;

        case cmdOPT_STOP:
            g_machineState.optStop = cmd.state();
            break;

		case cmdSEND_FILE_INIT:
			RecieveFileInit(cmd);
			break;

		case cmdSEND_FILE_DATA:
			RecieveFileData(cmd);
			break;
        }
    }
};


Sim::Sim()
{
    memset(&g_machineState, 0, sizeof(g_machineState));
}

void Sim::UpdateState()
{
	m_state.set_busy(g_machineState.busy ||
		g_machineState.running ||
		g_jogVel.x() != 0 ||
		g_jogVel.y() != 0 ||
		g_jogVel.z() != 0 ||
		g_jogVel.a() != 0 ||
		g_jogVel.b() != 0 ||
		g_jogVel.c() != 0);

    m_state.set_machine_connected(true);
    m_state.set_control_on(g_machineState.controlOn);
    CncRemote::Axes& axes = *m_state.mutable_abs_pos();

}

Connection * Sim::CreateConnection(CActiveSocket * client, Server * server)
{
    return new SimConnection(client, server);
}

bool Sim::Poll()
{
    if(g_machineState.busy)
	{
		g_machineState.busy --;
	}

	CncRemote::Axes& axes = g_curPos;
    axes.set_x(axes.x() + g_jogVel.x());
    axes.set_y(axes.y() + g_jogVel.y());
    axes.set_z(axes.z() + g_jogVel.z());
    axes.set_a(axes.a() + g_jogVel.a());
    axes.set_b(axes.b() + g_jogVel.b());
    axes.set_c(axes.c() + g_jogVel.c());
    if(g_machineState.running && !g_machineState.paused)
    {
        g_machineState.curLine++;
        if(g_machineState.step) g_machineState.paused = true;
    }
    return(Server::Poll());
}
