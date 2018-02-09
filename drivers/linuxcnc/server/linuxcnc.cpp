/****************************************************************
LinuxCNC server
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
#include "version.h"

/*
struct JOGAXIS
{
    hal_s32_t *counts;
    hal_bit_t *enable;
    hal_float_t *scale;
    hal_bit_t *velMode;
    hal_float_t *position;
};

JOGAXIS g_halAxes[MAX_AXES];*/
double g_jogAxes[MAX_AXES];
double g_maxSpeedLin = 1;
double g_maxSpeedAng = 1;

class LinuxConnection : public Connection
{
public:
    LinuxConnection(CActiveSocket * client, Server * server) : Connection(client, server)
    {
        SetTimeout(1);
    }

    virtual ~LinuxConnection()
    {
        LinuxCnc::ZeroJog(); //make sure we stop jogging if we lose connection
    }

    virtual void OnConnection(const CONNSTATE state)
    {
        MutexLocker l = m_server->GetLock();
        LinuxCnc::ZeroJog(); //make sure we stop jogging if we lose connection
    }

    virtual void HandlePacket(const Packet & pkt)
    {
        CncRemote::CmdBuf cmd;
        if(pkt.data.size() > 0 &&
                !cmd.ParseFromString(pkt.data))
        {
            OnPacketError();
            return;
        }
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
//        sendSetTeleopEnable(true);
            {
                const CncRemote::Axes& axes = cmd.axes();
                SendJogVel(axes.x(), axes.y(), axes.z(), axes.a(), axes.b(), axes.c());
            }
            break;

        case cmdJOGSTEP:
            SetMode(EMC_TASK_MODE_MANUAL);
            sendSetTeleopEnable(true);
            {
                const CncRemote::Axes& axes = cmd.axes();
                SendJogStep(0,axes.x());
                SendJogStep(1,axes.y());
                SendJogStep(2,axes.z());
                SendJogStep(3,axes.a());
                SendJogStep(4,axes.b());
                SendJogStep(5,axes.c());
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
            {
                EMC_TASK_PLAN_SET_BLOCK_DELETE emc_task_plan_set_block_delete_msg;
                emc_task_plan_set_block_delete_msg.state = cmd.state();
                emcCommandSend(emc_task_plan_set_block_delete_msg);
            }

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
            }
            else
            {
                sendFloodOff();
            }
            break;

        case cmdMIST:
            if(cmd.state())
            {
                sendMistOn();
            }
            else
            {
                sendMistOff();
            }
            break;

        }
    }

    static inline void SendJog(const int axis, const double vel)
    {

        if(vel == g_jogAxes[axis]) return;
        if(vel != 0)
        {
#if MAJOR_VER <= 2 && MINOR_VER <=8

            EMC_AXIS_JOG emc_axis_jog_msg;
            emc_axis_jog_msg.axis = axis;
            emc_axis_jog_msg.vel = vel;
            emcCommandSend(emc_axis_jog_msg);
#else
            EMC_JOG_CONT emc_jog_cont_msg;
            emc_jog_cont_msg.jjogmode = JOGTELEOP;
            emc_jog_cont_msg.joint_or_axis = axis;
            emc_jog_cont_msg.vel = vel;
            emcCommandSend(emc_jog_cont_msg);
#endif

        }
        else
        {
#if MAJOR_VER <= 2 && MINOR_VER <=8

            EMC_AXIS_ABORT emc_axis_abort_msg;
            emc_axis_abort_msg.axis = axis;
            emcCommandSend(emc_axis_abort_msg);
#else
            EMC_JOG_STOP emc_jog_stop_msg;
            emc_jog_stop_msg.jjogmode = JOGTELEOP;
            emc_jog_stop_msg.joint_or_axis = axis;
            emcCommandSend(emc_jog_stop_msg);
#endif
        }
        g_jogAxes[axis] = vel;
    }

    int SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c)
    {

        SendJog(0,x * g_maxSpeedLin);
        SendJog(1,y * g_maxSpeedLin);
        SendJog(2,z * g_maxSpeedLin);
        SendJog(3,a * g_maxSpeedAng);
        SendJog(4,b * g_maxSpeedAng);
        SendJog(5,c * g_maxSpeedAng);
        return 0;
    }


    void SendJogStep(const int axis, const double val)
    {
#if MAJOR_VER <= 2 && MINOR_VER <=8

        if(emcStatus->motion.axis[axis].axisType == EMC_AXIS_LINEAR)
            sendJogIncr(axis, val * emcStatus->motion.axis[axis].units, g_maxSpeedLin);
        else
            sendJogIncr(axis, val * emcStatus->motion.axis[axis].units, g_maxSpeedAng);
#else
        if(emcStatus->motion.joint[axis].jointType == EMC_LINEAR)
            sendJogIncr(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units, g_maxSpeedLin);
        else
            sendJogIncr(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units, g_maxSpeedAng);
#endif
    }


    void SetMode(const int mode)
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

};


LinuxCnc::LinuxCnc()
{
    m_slowCount = 0;
    m_heartbeat = 0;
    m_nextTime = 0;
    m_connected = false;

    g_maxSpeedLin = 4000;
    g_maxSpeedAng = 100;
    halId = -1;
    SetTimeout(0.005);
    memset(g_jogAxes, 0, sizeof(g_jogAxes));

}

void LinuxCnc::ConnectLCnc()
{
    while(tryNml(0.5,0.5) !=0)
    {
        m_state.set_machine_connected(false);
        Server::Poll();
    }

    // init NML
    // get current serial number, and save it for restoring when we quit
    // so as not to interfere with real operator interface
    updateStatus();

    if(emcStatus->size != sizeof(EMC_STAT))
    {
        printf("Wrong LinuxCNC version\n");
        exit(0);
    }

    emcCommandSerialNumber = emcStatus->echo_serial_number;
    m_heartbeat = emcStatus->task.heartbeat;
    m_nextTime = time(NULL) + 1; //check every second
    m_connected = true;

#define EMC_WAIT_NONE (EMC_WAIT_TYPE) 1
}


bool LinuxCnc::Poll()
{
    if(updateStatus() != 0)
    {
        printf("Disconnected\n");
        m_state.set_machine_connected(false);
        return false;
    }
    if(emcStatus->motion.traj.maxVelocity < 1e17)
    {
        g_maxSpeedLin = emcStatus->motion.traj.maxVelocity;
        g_maxSpeedAng = emcStatus->motion.traj.maxVelocity;
    }
    Server::Poll();
    return m_connected;
}

Connection * LinuxCnc::CreateConnection(CActiveSocket * client, Server * server)
{
    return new LinuxConnection(client, server);
}

void LinuxCnc::UpdateState()
{
    m_state.Clear();
    if(emcStatus == NULL)
    {
        m_state.set_machine_connected(false);
        return;
    }
    CncRemote::Axes& axes = *m_state.mutable_abs_pos();
#if MAJOR_VER <= 2 && MINOR_VER <=8
    axes.set_x(emcStatus->motion.traj.actualPosition.tran.x / emcStatus->motion.axis[0].units);
    axes.set_y(emcStatus->motion.traj.actualPosition.tran.y / emcStatus->motion.axis[1].units);
    axes.set_z(emcStatus->motion.traj.actualPosition.tran.z / emcStatus->motion.axis[2].units);
    axes.set_a(emcStatus->motion.traj.actualPosition.a / emcStatus->motion.axis[3].units);
    axes.set_b(emcStatus->motion.traj.actualPosition.b / emcStatus->motion.axis[4].units);
    axes.set_c(emcStatus->motion.traj.actualPosition.c / emcStatus->motion.axis[5].units);
#else
    axes.set_x(emcStatus->motion.traj.actualPosition.tran.x / emcStatus->motion.joint[0].units);
    axes.set_y(emcStatus->motion.traj.actualPosition.tran.y / emcStatus->motion.joint[1].units);
    axes.set_z(emcStatus->motion.traj.actualPosition.tran.z / emcStatus->motion.joint[2].units);
    axes.set_a(emcStatus->motion.traj.actualPosition.a / emcStatus->motion.joint[3].units);
    axes.set_b(emcStatus->motion.traj.actualPosition.b / emcStatus->motion.joint[4].units);
    axes.set_c(emcStatus->motion.traj.actualPosition.c / emcStatus->motion.joint[5].units);
#endif

    switch (m_slowCount++) //these don't need to be updated very fast so only send one per frame
    {
    case 1:
    {
        static bool prevState = false;
        bool state = emcStatus->task.state == EMC_TASK_STATE_ON;
        if(state != prevState)
        {
            ZeroJog();
            prevState = state;
        }
        m_state.set_control_on(state);
    }
    m_state.set_machine_connected(true);
    break;

    case 2:
        switch(emcStatus->motion.spindle.enabled)
        {
        case 0:
            m_state.set_spindle_state(CncRemote::spinOFF);
            break;

        case -1:
            m_state.set_spindle_state(CncRemote::spinREV);
            break;

        case 1:
            m_state.set_spindle_state(CncRemote::spinFWD);
            break;
        }
        m_state.set_spindle_speed(emcStatus->motion.spindle.speed);
        break;

    case 3:
        m_state.set_paused(emcStatus->task.task_paused);
#if MAJOR_VER <= 2 && MINOR_VER <=8
        m_state.set_max_feed_lin((g_maxSpeedLin * 60) / emcStatus->motion.traj.linearUnits);
        m_state.set_max_feed_ang((g_maxSpeedAng * 60) / emcStatus->motion.traj.angularUnits);
#else
        set_max_feed_lin((g_maxSpeedLin * 60) / emcStatus->motion.traj.linearUnits);
        set_max_feed_ang((g_maxSpeedAng * 60) / emcStatus->motion.traj.angularUnits);
#endif
        break;

    case 4:
        m_state.set_optional_stop(emcStatus->task.optional_stop_state);
        m_state.set_block_delete(emcStatus->task.block_delete_state);
        break;

    case 5:
        m_state.set_mist(emcStatus->io.coolant.mist);
        m_state.set_flood(emcStatus->io.coolant.flood);
        break;

    case 6:
    {
        static bool prevState = false;
        bool state = emcStatus->task.interpState != EMC_TASK_INTERP_IDLE;
        if(state != prevState)
        {
            ZeroJog();
            prevState = state;
        }
        m_state.set_running(state);
    }

    updateError();
    if(error_string[0] != 0)
    {
        m_state.set_error_msg(error_string);
        error_string[0] = 0;
    }
    if(operator_text_string[0] != 0)
    {
        m_state.set_display_msg(operator_text_string);
        operator_text_string[0] = 0;
    }
    else if(operator_display_string[0] != 0)
    {
        m_state.set_display_msg(operator_display_string);
        operator_display_string[0] = 0;
    }
    break;

    case 7:
        m_state.set_current_line(emcStatus->task.motionLine);
        break;

    case 8:
    {
        CncRemote::Axes& axes = *m_state.mutable_offset_fixture();
#if MAJOR_VER <= 2 && MINOR_VER <=8
        axes.set_x(emcStatus->task.g5x_offset.tran.x / emcStatus->motion.axis[0].units);
        axes.set_y(emcStatus->task.g5x_offset.tran.y / emcStatus->motion.axis[1].units);
        axes.set_z(emcStatus->task.g5x_offset.tran.z / emcStatus->motion.axis[2].units);
        axes.set_a(emcStatus->task.g5x_offset.a / emcStatus->motion.axis[3].units);
        axes.set_b(emcStatus->task.g5x_offset.b / emcStatus->motion.axis[4].units);
        axes.set_c(emcStatus->task.g5x_offset.c / emcStatus->motion.axis[5].units);
#else
        axes.set_x(emcStatus->task.g5x_offset.tran.x / emcStatus->motion.joint[0].units);
        axes.set_y(emcStatus->task.g5x_offset.tran.y / emcStatus->motion.joint[1].units);
        axes.set_z(emcStatus->task.g5x_offset.tran.z / emcStatus->motion.joint[2].units);
        axes.set_a(emcStatus->task.g5x_offset.a / emcStatus->motion.joint[3].units);
        axes.set_b(emcStatus->task.g5x_offset.b / emcStatus->motion.joint[4].units);
        axes.set_c(emcStatus->task.g5x_offset.c / emcStatus->motion.joint[5].units);
#endif
    }
    break;

    case 9:
    {
        CncRemote::Axes& axes = *m_state.mutable_offset_work();
#if MAJOR_VER <= 2 && MINOR_VER <=8
        axes.set_x(emcStatus->task.g92_offset.tran.x / emcStatus->motion.axis[0].units);
        axes.set_y(emcStatus->task.g92_offset.tran.y / emcStatus->motion.axis[1].units);
        axes.set_z(emcStatus->task.g92_offset.tran.z / emcStatus->motion.axis[2].units);
        axes.set_a(emcStatus->task.g92_offset.a / emcStatus->motion.axis[3].units);
        axes.set_b(emcStatus->task.g92_offset.b / emcStatus->motion.axis[4].units);
        axes.set_c(emcStatus->task.g92_offset.c / emcStatus->motion.axis[5].units);
#else
        axes.set_x(emcStatus->task.g92_offset.tran.x / emcStatus->motion.joint[0].units);
        axes.set_y(emcStatus->task.g92_offset.tran.y / emcStatus->motion.joint[1].units);
        axes.set_z(emcStatus->task.g92_offset.tran.z / emcStatus->motion.joint[2].units);
        axes.set_a(emcStatus->task.g92_offset.a / emcStatus->motion.joint[3].units);
        axes.set_b(emcStatus->task.g92_offset.b / emcStatus->motion.joint[4].units);
        axes.set_c(emcStatus->task.g92_offset.c / emcStatus->motion.joint[5].units);
#endif
    }
    break;

    case 10:
    {
        CncRemote::BoolAxes& axes = *m_state.mutable_homed();
#if MAJOR_VER <= 2 && MINOR_VER <=8
        axes.set_x(emcStatus->motion.axis[0].homed);
        axes.set_y(emcStatus->motion.axis[1].homed);
        axes.set_z(emcStatus->motion.axis[2].homed);
        axes.set_a(emcStatus->motion.axis[3].homed);
        axes.set_b(emcStatus->motion.axis[4].homed);
        axes.set_c(emcStatus->motion.axis[5].homed);
#else
        axes.set_x(emcStatus->motion.joint[0].homed);
        axes.set_y(emcStatus->motion.joint[1].homed);
        axes.set_z(emcStatus->motion.joint[2].homed);
        axes.set_a(emcStatus->motion.joint[3].homed);
        axes.set_b(emcStatus->motion.joint[4].homed);
        axes.set_c(emcStatus->motion.joint[5].homed);
#endif
    }
    break;

    case 11:
        m_state.set_gcode_units(convertLinearUnits(emcStatus->motion.traj.linearUnits));
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


void LinuxCnc::ZeroJog()
{
    for(int ct=0; ct < MAX_AXES; ct++)
    {
        LinuxConnection::SendJog(ct, 0);
    }
}
