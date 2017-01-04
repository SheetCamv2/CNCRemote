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
    axes.set_x(emcStatus->motion.traj.actualPosition.tran.x / emcStatus->motion.joint[0].units);
    axes.set_y(emcStatus->motion.traj.actualPosition.tran.y / emcStatus->motion.joint[1].units);
    axes.set_z(emcStatus->motion.traj.actualPosition.tran.z / emcStatus->motion.joint[2].units);
    axes.set_a(emcStatus->motion.traj.actualPosition.a / emcStatus->motion.joint[3].units);
    axes.set_b(emcStatus->motion.traj.actualPosition.b / emcStatus->motion.joint[4].units);
    axes.set_c(emcStatus->motion.traj.actualPosition.c / emcStatus->motion.joint[5].units);

    switch (m_slowCount++) //these don't need to be updated very fast so only send one per frame
    {
    case 1:
        set_control_on(emcStatus->task.state == EMC_TASK_STATE_ON );
        set_machine_connected(true);
        break;

    case 2:
        switch(emcStatus->motion.spindle.enabled)
        {
        case 0:
            set_spindle_state(CncRemote::spinOFF);
            break;

        case -1:
            set_spindle_state(CncRemote::spinREV);
            break;

        case 1:
            set_spindle_state(CncRemote::spinFWD);
            break;
        }
        set_spindle_speed(emcStatus->motion.spindle.speed);
        break;

    case 3:
        set_paused(emcStatus->task.task_paused);
        break;

    case 4:
        set_optional_stop(emcStatus->task.optional_stop_state);
        set_block_delete(emcStatus->task.block_delete_state);
        break;

    case 5:
        set_mist(emcStatus->io.coolant.mist);
        set_flood(emcStatus->io.coolant.flood);
        break;

    case 6:
        set_running(emcStatus->task.interpState != EMC_TASK_INTERP_IDLE);
        if(error_string[0] != 0)
        {
            set_error_msg(error_string);
            error_string[0] = 0;
        }
        if(operator_text_string[0] != 0)
        {
            set_display_msg(operator_text_string);
            operator_text_string[0] = 0;
        }else if(operator_display_string[0] != 0)
        {
            set_display_msg(operator_display_string);
            operator_display_string[0] = 0;
        }
        break;

    case 7:
        set_current_line(emcStatus->task.motionLine);
        break;

    case 8:
    {
        CncRemote::Axes& axes = *mutable_offset_fixture();
        axes.set_x(emcStatus->task.g5x_offset.tran.x / emcStatus->motion.joint[0].units);
        axes.set_y(emcStatus->task.g5x_offset.tran.y / emcStatus->motion.joint[1].units);
        axes.set_z(emcStatus->task.g5x_offset.tran.z / emcStatus->motion.joint[2].units);
        axes.set_a(emcStatus->task.g5x_offset.a / emcStatus->motion.joint[3].units);
        axes.set_b(emcStatus->task.g5x_offset.b / emcStatus->motion.joint[4].units);
        axes.set_c(emcStatus->task.g5x_offset.c / emcStatus->motion.joint[5].units);
    }
    break;

    case 9:
    {
        CncRemote::Axes& axes = *mutable_offset_work();
        axes.set_x(emcStatus->task.g92_offset.tran.x / emcStatus->motion.joint[0].units);
        axes.set_y(emcStatus->task.g92_offset.tran.y / emcStatus->motion.joint[1].units);
        axes.set_z(emcStatus->task.g92_offset.tran.z / emcStatus->motion.joint[2].units);
        axes.set_a(emcStatus->task.g92_offset.a / emcStatus->motion.joint[3].units);
        axes.set_b(emcStatus->task.g92_offset.b / emcStatus->motion.joint[4].units);
        axes.set_c(emcStatus->task.g92_offset.c / emcStatus->motion.joint[5].units);
    }
    break;

    case 10:
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
        sendSetTeleopEnable(true);
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
        break;

    case cmdHOME:
    {
        SetMode(EMC_TASK_MODE_MANUAL);
        sendSetTeleopEnable(false);
        const CncRemote::BoolAxes& axes = cmd.bool_axes();
        if(axes.x() && axes.y() && axes.z())
        {
            sendHome(-1); //home all
            break;
        }
        if(axes.x())
        {
            sendHome(0);
            break;
        }
        if(axes.y())
        {
            sendHome(1);
            break;
        }
        if(axes.z())
        {
            sendHome(2);
            break;
        }
        if(axes.a())
        {
            sendHome(3);
            break;
        }
        if(axes.b())
        {
            sendHome(4);
            break;
        }
        if(axes.c())
        {
            sendHome(5);
            break;
        }
//        sendSetTeleopEnable(true);
    }
    break;

    case cmdSPINDLE:
        switch(cmd.intval())
        {
        case CncRemote::spinOFF:
            sendSpindleOff();
            break;

        case CncRemote::spinFWD:
            sendSpindleForward();
            break;

        case CncRemote::spinREV:
                sendSpindleReverse();
            break;
        }
        break;


    case cmdFLOOD:
        if(cmd.state())
        {
            sendFloodOn();
        }else
        {
            sendFloodOff();
        }
        break;

    case cmdMIST:
        if(cmd.state())
        {
            sendMistOn();
        }else
        {
            sendMistOff();
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


