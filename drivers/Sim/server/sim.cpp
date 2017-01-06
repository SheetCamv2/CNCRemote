#include "sim.h"


const char *g_cmdNames[]={
    "cmdNULL",
    "cmdPING",
    "cmdSTATE",
    "cmdDRIVESON",
    "cmdJOGVEL",
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
    "cmdSENDFILE",
    "cmdREQFILE",
	"cmdCOOLANT",
    "cmdMIST",
    "cmdSPINDLE",
    "cmdHOME",
};


void Sim::HandlePacket(const Packet & pkt)
{
    if(pkt.cmd < cmdMAX)
    {
        printf("Command %d %s\n", pkt.cmd, g_cmdNames[pkt.cmd]);
    }

    CncRemote::CmdBuf cmd;
    cmd.ParseFromString(pkt.data);
    switch(pkt.cmd)
    {
    case cmdDRIVESON:
        set_control_on(cmd.state());
        break;

    case cmdJOGVEL:
        m_jogVel = cmd.axes();
        break;

    case cmdMDI:
        printf("MDI:%s\n", cmd.string().c_str());
        break;

    case cmdFRO:
        set_feed_override(cmd.rate());
        break;

    case cmdFILE:
        break;

    case cmdCLOSEFILE:
        break;

    case cmdSTART:
        if(paused())
        {
            set_paused(false);
            break;
        }
        set_running(true);
        break;

    case cmdSTOP:
        set_running(false);
        break;

    case cmdPAUSE:
        set_paused(true);
        break;

    case cmdBLOCKDEL:
        set_block_delete(cmd.state());
        break;

    case cmdSINGLESTEP:
        set_single_step(cmd.state());
        break;

    case cmdOPTSTOP:
        set_optional_stop(cmd.state());
        break;
    }
}

void Sim::UpdateState()
{
    set_machine_connected(true);
    set_control_on(true);
    CncRemote::Axes& axes = *mutable_max_feed();
    axes.set_x(1);
    axes.set_y(1);
    axes.set_z(1);
    axes.set_a(1);
    axes.set_b(1);
    axes.set_c(1);
}

bool Sim::Poll()
{
    CncRemote::Axes& axes = *mutable_abs_pos();
    axes.set_x(axes.x() + m_jogVel.x());
    axes.set_y(axes.y() + m_jogVel.y());
    axes.set_z(axes.z() + m_jogVel.z());
    axes.set_a(axes.a() + m_jogVel.a());
    axes.set_b(axes.b() + m_jogVel.b());
    axes.set_c(axes.c() + m_jogVel.c());
    if(running() && !paused())
    {
        set_current_line(current_line() + 1);
        if(single_step()) set_paused(true);
    }
    return(Server::Poll());
}
