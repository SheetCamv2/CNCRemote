#include "linuxcnc.h"


#include "rcs.hh"
#include "posemath.h"		// PM_POSE, TO_RAD
#include "emc.hh"		// EMC NML
#include "canon.hh"		// CANON_UNITS, CANON_UNITS_INCHES,MM,CM
#include "emcglb.h"		// EMC_NMLFILE, TRAJ_MAX_VELOCITY, etc.
#include "emccfg.h"		// DEFAULT_TRAJ_MAX_VELOCITY
#include "inifile.hh"		// INIFILE
#include "rcs_print.hh"
#include "timer.hh"             // etime()
#include "shcom.hh"             // NML Messaging functions

#include "shcom.cc" //this way we can use the include search path to find shcom.cc

LinuxCnc::LinuxCnc()
{
    m_slowCount = 0;
    m_heartbeat = 0;
    m_nextTime = 0;
    m_connected = false;
}

void LinuxCnc::ConnectLCnc()
{
    while(tryNml(0.5,0.5) !=0)
    {
        set_machine_connected(false);
        Server::Poll();
    }
    // init NML
    // get current serial number, and save it for restoring when we quit
    // so as not to interfere with real operator interface
    updateStatus();
    emcCommandSerialNumber = emcStatus->echo_serial_number;
    m_heartbeat = emcStatus->task.heartbeat;
    m_nextTime = time(NULL) + 1; //check every second
    m_connected = true;
}

bool LinuxCnc::Poll()
{
    if(updateStatus() != 0)
    {
        Disconnect();
        return false;
    }
    Server::Poll();
    return m_connected;
}

void LinuxCnc::UpdateState()
{
    Clear();
    if(emcStatus == NULL)
    {
        set_machine_connected(false);
        return;
    }
    CncRemote::Axes& axes = *mutable_abs_pos();
    axes.set_x(emcStatus->motion.traj.actualPosition.tran.x);
    axes.set_y(emcStatus->motion.traj.actualPosition.tran.y);
    axes.set_z(emcStatus->motion.traj.actualPosition.tran.z);
    axes.set_a(emcStatus->motion.traj.actualPosition.a);
    axes.set_b(emcStatus->motion.traj.actualPosition.b);
    axes.set_c(emcStatus->motion.traj.actualPosition.c);

    switch (m_slowCount++) //these don't need to be updated very fast so only send one per frame
    {
    case 1:
        set_control_on(emcStatus->task.state == EMC_TASK_STATE_ON );
        break;

    case 2:
        set_machine_connected(true);
        break;

    case 3:
        set_paused(emcStatus->task.task_paused);
        break;

    case 4:
        set_optional_stop(emcStatus->task.optional_stop_state);
        break;

    case 5:
        set_block_delete(emcStatus->task.block_delete_state);
        break;

    case 6:
        set_running(emcStatus->task.interpState != EMC_TASK_INTERP_IDLE);
        break;

    case 7:
        set_current_line(emcStatus->task.motionLine);
        break;

    case 8:
        set_lin_unit_scale(emcStatus->motion.traj.linearUnits);
        break;

    case 9:
    {
        CncRemote::Axes& axes = *mutable_offset_fixture();
        axes.set_x(emcStatus->task.g5x_offset.tran.x);
        axes.set_y(emcStatus->task.g5x_offset.tran.y);
        axes.set_z(emcStatus->task.g5x_offset.tran.z);
        axes.set_a(emcStatus->task.g5x_offset.a);
        axes.set_b(emcStatus->task.g5x_offset.b);
        axes.set_c(emcStatus->task.g5x_offset.c);
    }
    break;

    case 10:
    {
        CncRemote::Axes& axes = *mutable_offset_work();
        axes.set_x(emcStatus->task.g92_offset.tran.x);
        axes.set_y(emcStatus->task.g92_offset.tran.y);
        axes.set_z(emcStatus->task.g92_offset.tran.z);
        axes.set_a(emcStatus->task.g92_offset.a);
        axes.set_b(emcStatus->task.g92_offset.b);
        axes.set_c(emcStatus->task.g92_offset.c);
    }
    break;

    case 11:
    {
        CncRemote::BoolAxes& axes = *mutable_homed();
        axes.set_x(emcStatus->motion.joint[0].homed);
        axes.set_y(emcStatus->motion.joint[1].homed);
        axes.set_z(emcStatus->motion.joint[2].homed);
        axes.set_a(emcStatus->motion.joint[3].homed);
        axes.set_b(emcStatus->motion.joint[4].homed);
        axes.set_c(emcStatus->motion.joint[5].homed);
    }
    break;

    default:
        if(time(NULL) > m_nextTime)
        {
            if(m_heartbeat != emcStatus->task.heartbeat)
            {
                m_heartbeat = emcStatus->task.heartbeat;
            }
            else
            {
                m_connected = false;
            }
            m_nextTime = time(NULL) + 1; //check every second
        }
        m_slowCount = 1;
    }
}



void LinuxCnc::HandlePacket(const Packet & pkt)
{
    CncRemote::CmdBuf cmd;
    cmd.ParseFromString(pkt.data);
    if(emcStatus == NULL) return;
    switch(pkt.cmd)
    {
    case cmdDRIVESON:
        if(cmd.state())
        {
            sendEstopReset();
            sendMachineOn();
        }
        else
        {
            sendEstop();
        }
        break;

    case cmdJOGVEL:
        SetMode(EMC_TASK_MODE_MANUAL);
        {
            const CncRemote::Axes& axes = cmd.axes();
            sendJogCont(0,JOGTELEOP,axes.x());
            sendJogCont(1,JOGTELEOP,axes.y());
            sendJogCont(2,JOGTELEOP,axes.z());
            sendJogCont(3,JOGTELEOP,axes.a());
            sendJogCont(4,JOGTELEOP,axes.b());
            sendJogCont(5,JOGTELEOP,axes.c());
        }
        break;

    case cmdMDI:
        SetMode(EMC_TASK_MODE_MDI);
        sendMdiCmd(cmd.string().c_str());
        break;

    case cmdFRO:
        sendFeedOverride(cmd.rate());
        break;

    case cmdFILE:
        sendProgramOpen((char *)(cmd.string().c_str()));
        break;

    case cmdSTART:
        SetMode(EMC_TASK_MODE_AUTO);
        sendProgramRun(0);
        break;

    case cmdSTOP:
        sendAbort();
        break;

    case cmdPAUSE:
        if(cmd.state())
        {
            sendProgramPause();
        }
        else
        {
            sendProgramResume();
        }
        break;

    case cmdBLOCKDEL:
        break;

    case cmdSINGLESTEP:
        sendProgramStep();
        break;

    case cmdOPTSTOP:
        sendSetOptionalStop(cmd.state());

    case cmdHOME:
    {
        SetMode(EMC_TASK_MODE_MANUAL);
        const CncRemote::BoolAxes& axes = cmd.bool_axes();
        if(axes.z()) sendHome(2);
        if(axes.x()) sendHome(0);
        if(axes.y()) sendHome(1);
        if(axes.a()) sendHome(3);
        if(axes.b()) sendHome(4);
        if(axes.c()) sendHome(5);
    }

        break;
    }
}

void LinuxCnc::SetMode(const int mode)
{
    if(emcStatus->task.mode == mode) return;
    switch(mode)
    {
    case EMC_TASK_MODE_MANUAL:
        sendManual();
        break;

    case EMC_TASK_MODE_AUTO:
        sendAuto();
        break;

    case EMC_TASK_MODE_MDI:
        sendMdi();
        break;
    }
}
