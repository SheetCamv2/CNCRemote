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

LinuxCnc::LinuxCnc()
{
    m_slowCount = 0;
    m_heartbeat = 0;
    m_connected = false;

    g_maxSpeedLin = 4000;
    g_maxSpeedAng = 100;
    halId = -1;
    memset(g_jogAxes, 0, sizeof(g_jogAxes));

}

void LinuxCnc::ConnectLCnc()
{
/*    linear::mutex& mutex = GetMutex();
    mutex.unlock();
    {
        LockedState s = GetState();
        s->machineState = mcOFFLINE;
    }
    mutex.lock();*/


    while(tryNml(0.5, 0.5) !=0)
    {
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

    if(emcStatus->motion.traj.maxVelocity < 1e17)
    {
        g_maxSpeedLin = emcStatus->motion.traj.maxVelocity;
        g_maxSpeedAng = emcStatus->motion.traj.maxVelocity;
    }

    m_connected = true;

#define EMC_WAIT_NONE (EMC_WAIT_TYPE) 1
}


bool LinuxCnc::Poll()
{
    return m_connected;
}


void LinuxCnc::UpdateState(State& state)
{
    if(emcStatus == NULL)
    {
        state.machineState = mcOFFLINE;
        return;
    }


    if(updateStatus() != 0)
    {
        printf("Disconnected\n");
        state.machineState = mcOFFLINE;
        m_connected = false;
        return;
    }


    if(time(NULL) > m_nextTime)
    {
        if(m_heartbeat != emcStatus->task.heartbeat)
        {
            m_heartbeat = emcStatus->task.heartbeat;
        }else
        {
            printf("Disconnected\n");
            state.machineState = mcOFFLINE;
            m_connected = false;
            return;
        }
        m_nextTime = time(NULL) + 1; //check every second
    }


    {
        CncRemote::Axes& axes = state.machinePos;
#if LINUXCNC_PRE_JOINTS
        axes.x = emcStatus->motion.traj.actualPosition.tran.x / emcStatus->motion.axis[0].units;
        axes.y = emcStatus->motion.traj.actualPosition.tran.y / emcStatus->motion.axis[1].units;
        axes.z = emcStatus->motion.traj.actualPosition.tran.z / emcStatus->motion.axis[2].units;
        axes.a = emcStatus->motion.traj.actualPosition.a / emcStatus->motion.axis[3].units;
        axes.b = emcStatus->motion.traj.actualPosition.b / emcStatus->motion.axis[4].units;
        axes.c = emcStatus->motion.traj.actualPosition.c / emcStatus->motion.axis[5].units;
#else
        axes.x = emcStatus->motion.traj.actualPosition.tran.x / emcStatus->motion.joint[0].units;
        axes.y = emcStatus->motion.traj.actualPosition.tran.y / emcStatus->motion.joint[1].units;
        axes.z = emcStatus->motion.traj.actualPosition.tran.z / emcStatus->motion.joint[2].units;
        axes.a = emcStatus->motion.traj.actualPosition.a / emcStatus->motion.joint[3].units;
        axes.b = emcStatus->motion.traj.actualPosition.b / emcStatus->motion.joint[4].units;
        axes.c = emcStatus->motion.traj.actualPosition.c / emcStatus->motion.joint[5].units;
#endif
    }

    {
        CncRemote::Axes& axes = state.position;


#if LINUXCNC_PRE_JOINTS
        axes.x = (emcStatus->motion.traj.actualPosition.tran.x - emcStatus->task.g5x_offset.tran.x - emcStatus->task.g92_offset.tran.x) / emcStatus->motion.axis[0].units;
        axes.y = (emcStatus->motion.traj.actualPosition.tran.y - emcStatus->task.g5x_offset.tran.y - emcStatus->task.g92_offset.tran.y) / emcStatus->motion.axis[1].units;
        axes.z = (emcStatus->motion.traj.actualPosition.tran.z - emcStatus->task.g5x_offset.tran.z - emcStatus->task.g92_offset.tran.z) / emcStatus->motion.axis[2].units;
        axes.a = (emcStatus->motion.traj.actualPosition.a      - emcStatus->task.g5x_offset.a      - emcStatus->task.g92_offset.a)      / emcStatus->motion.axis[3].units;
        axes.b = (emcStatus->motion.traj.actualPosition.b      - emcStatus->task.g5x_offset.b      - emcStatus->task.g92_offset.b)      / emcStatus->motion.axis[4].units;
        axes.c = (emcStatus->motion.traj.actualPosition.c      - emcStatus->task.g5x_offset.c      - emcStatus->task.g92_offset.c)      / emcStatus->motion.axis[5].units;
#else
        axes.x = (emcStatus->motion.traj.actualPosition.tran.x - emcStatus->task.g5x_offset.tran.x - emcStatus->task.g92_offset.tran.x) / emcStatus->motion.joint[0].units;
        axes.y = (emcStatus->motion.traj.actualPosition.tran.y - emcStatus->task.g5x_offset.tran.y - emcStatus->task.g92_offset.tran.y) / emcStatus->motion.joint[1].units;
        axes.z = (emcStatus->motion.traj.actualPosition.tran.z - emcStatus->task.g5x_offset.tran.z - emcStatus->task.g92_offset.tran.z) / emcStatus->motion.joint[2].units;
        axes.a = (emcStatus->motion.traj.actualPosition.a      - emcStatus->task.g5x_offset.a      - emcStatus->task.g92_offset.a)      / emcStatus->motion.joint[3].units;
        axes.b = (emcStatus->motion.traj.actualPosition.b      - emcStatus->task.g5x_offset.b      - emcStatus->task.g92_offset.b)      / emcStatus->motion.joint[4].units;
        axes.c = (emcStatus->motion.traj.actualPosition.c      - emcStatus->task.g5x_offset.c      - emcStatus->task.g92_offset.c)      / emcStatus->motion.joint[5].units;
#endif
    }




    if(emcStatus->task.state == EMC_TASK_STATE_ON)
    {

        if(emcStatus->motion.traj.current_vel == 0)
        {
            state.machineState = mcIDLE;
        }else
        {
            state.machineState = mcMOVING;
        }
        if(emcStatus->task.interpState > EMC_TASK_INTERP_IDLE)
        {
            if(emcStatus->task.mode == EMC_TASK_MODE_MDI)
            {
                state.machineState = mcMDI;
            }else
            {
                state.machineState = mcRUNNING;
            }
        }

    }else
    {
        state.machineState = mcOFF;
    }

    if(state.machineState != mcIDLE) ZeroJog();

#if LINUXCNC_MULTI_SPINDLE
    switch(emcStatus->motion.spindle[0].enabled)
#else
    switch(emcStatus->motion.spindle.enabled)
#endif
    {
    case 0:
        state.spindleState = CncRemote::spinOFF;
        break;

    case -1:
        state.spindleState = CncRemote::spinREV;
        break;

    case 1:
        state.spindleState = CncRemote::spinFWD;
        break;
    }
#if LINUXCNC_MULTI_SPINDLE
    state.spindleCmd = emcStatus->motion.spindle[0].speed;
    state.spindleActual = emcStatus->motion.spindle[0].speed; //FIXME: Should be actual spindle speed
#else
    state.spindleCmd = emcStatus->motion.spindle.speed;
    state.spindleActual = emcStatus->motion.spindle.speed; //FIXME: Should be actual spindle speed
#endif

    state.feedHold = emcStatus->task.task_paused;
#if LINUXCNC_PRE_JOINTS
    state.maxFeedLin = ((g_maxSpeedLin * 60) / emcStatus->motion.traj.linearUnits);
    state.maxFeedAng = ((g_maxSpeedAng * 60) / emcStatus->motion.traj.angularUnits);
#else
    state.maxFeedLin = ((g_maxSpeedLin * 60) / emcStatus->motion.traj.linearUnits);
    state.maxFeedAng = ((g_maxSpeedAng * 60) / emcStatus->motion.traj.angularUnits);
#endif
    state.optionalStop = emcStatus->task.optional_stop_state;
    state.blockDelete = emcStatus->task.block_delete_state;
    state.mist = emcStatus->io.coolant.mist;
    state.flood = emcStatus->io.coolant.flood;

    updateError();
    if(error_string[0] != 0)
    {
        LogError(error_string);
        error_string[0] = 0;
    }
    if(operator_text_string[0] != 0)
    {
        LogMessage(operator_text_string);
        operator_text_string[0] = 0;
    }
    else if(operator_display_string[0] != 0)
    {
        LogMessage(operator_display_string);
        operator_display_string[0] = 0;
    }
    state.currentLine = emcStatus->task.motionLine;

    {
        CncRemote::BoolAxes& axes = state.homed;
#if LINUXCNC_PRE_JOINTS
        axes.x = emcStatus->motion.axis[0].homed;
        axes.y = emcStatus->motion.axis[1].homed;
        axes.z = emcStatus->motion.axis[2].homed;
        axes.a = emcStatus->motion.axis[3].homed;
        axes.b = emcStatus->motion.axis[4].homed;
        axes.c = emcStatus->motion.axis[5].homed;
#else
        axes.x = emcStatus->motion.joint[0].homed;
        axes.y = emcStatus->motion.joint[1].homed;
        axes.z = emcStatus->motion.joint[2].homed;
        axes.a = emcStatus->motion.joint[3].homed;
        axes.b = emcStatus->motion.joint[4].homed;
        axes.c = emcStatus->motion.joint[5].homed;
#endif
    }
    state.gcodeUnits = convertLinearUnits(emcStatus->motion.traj.linearUnits);
}


void LinuxCnc::ZeroJog()
{
/*    for(int ct=0; ct < MAX_AXES; ct++)
    {
        LinuxConnection::SendJog(ct, 0);
    }*/
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

void LinuxCnc::SendJog(const int axis, const double vel)
{

    if(vel == g_jogAxes[axis]) return;
    if(vel != 0)
    {
#if LINUXCNC_PRE_JOINTS

        EMC_AXIS_JOG emc_axis_jog_msg;
        emc_axis_jog_msg.axis = axis;
        emc_axis_jog_msg.vel = vel;
        emcCommandSend(emc_axis_jog_msg);
#else
        EMC_JOG_CONT emc_jog_cont_msg;
        emc_jog_cont_msg.jjogmode = JOGTELEOP;
        if(emcStatus->motion.traj.mode == EMC_TRAJ_MODE_FREE) emc_jog_cont_msg.jjogmode = JOGJOINT;
        emc_jog_cont_msg.joint_or_axis = axis;
        emc_jog_cont_msg.vel = vel;
        emcCommandSend(emc_jog_cont_msg);
#endif

    }
    else
    {
#if LINUXCNC_PRE_JOINTS

        EMC_AXIS_ABORT emc_axis_abort_msg;
        emc_axis_abort_msg.axis = axis;
        emcCommandSend(emc_axis_abort_msg);
#else
        EMC_JOG_STOP emc_jog_stop_msg;
        emc_jog_stop_msg.jjogmode = JOGTELEOP;
        if(emcStatus->motion.traj.mode == EMC_TRAJ_MODE_FREE) emc_jog_stop_msg.jjogmode = JOGJOINT;
        emc_jog_stop_msg.joint_or_axis = axis;
        emcCommandSend(emc_jog_stop_msg);
#endif
    }
    g_jogAxes[axis] = vel;
}

int LinuxCnc::SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c)
{

    SendJog(0,x * g_maxSpeedLin);
    SendJog(1,y * g_maxSpeedLin);
    SendJog(2,z * g_maxSpeedLin);
    SendJog(3,a * g_maxSpeedAng);
    SendJog(4,b * g_maxSpeedAng);
    SendJog(5,c * g_maxSpeedAng);
    return 0;
}


void LinuxCnc::SendJogStep(const int axis, const double val)
{
#if LINUXCNC_PRE_JOINTS

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




void LinuxCnc::DrivesOn(const bool state)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    if(state)
    {
        sendEstopReset();
        sendMachineOn();
    }
    else
    {
        sendEstop();
    }
}

void LinuxCnc::JogVel(const Axes velocities)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    SendJogVel(velocities.x, velocities.y, velocities.z, velocities.a, velocities.b, velocities.c);
}

void LinuxCnc::JogStep(const Axes distance, const double speed)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    sendSetTeleopEnable(true);
    if(distance.x != 0) SendJogStep(0,distance.x);
    if(distance.y != 0) SendJogStep(1,distance.y);
    if(distance.z != 0) SendJogStep(2,distance.z);
    if(distance.a != 0) SendJogStep(3,distance.a);
    if(distance.b != 0) SendJogStep(4,distance.b);
    if(distance.c != 0) SendJogStep(5,distance.c);
}

bool LinuxCnc::Mdi(const string line)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return false;
    SetMode(EMC_TASK_MODE_MDI);
    return sendMdiCmd(line.c_str()) >= 0;
}

void LinuxCnc::SpindleOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
#if LINUXCNC_MULTI_SPINDLE
    sendSpindleOverride(0,percent);
#else
    sendSpindleOverride(percent);
#endif
}

void LinuxCnc::FeedOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    sendFeedOverride(percent);
}

void LinuxCnc::RapidOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    sendRapidOverride(percent);
}

bool LinuxCnc::LoadFile(const string fileName)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return false;
    return sendProgramOpen((char *)fileName.c_str()) >= 0;
}

bool LinuxCnc::CloseFile()
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return false;
    return sendProgramOpen((char *)"") >= 0;
}

void LinuxCnc::CycleStart()
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    SetMode(EMC_TASK_MODE_AUTO);
    sendProgramRun(0);
}

void LinuxCnc::CycleStop()
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    sendAbort();
}

void LinuxCnc::FeedHold(const bool state)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    if(state)
    {
        sendProgramPause();
    }
    else
    {
        sendProgramResume();
    }
}

void LinuxCnc::BlockDelete(const bool state)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    EMC_TASK_PLAN_SET_BLOCK_DELETE emc_task_plan_set_block_delete_msg;
    emc_task_plan_set_block_delete_msg.state = state;
    emcCommandSend(emc_task_plan_set_block_delete_msg);
}

void LinuxCnc::SingleStep(const bool state)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    sendProgramStep();
}

void LinuxCnc::OptionalStop(const bool state)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    sendSetOptionalStop(state);
}

void LinuxCnc::Home(const BoolAxes axes)
{
    ThreadLock lock = GetLock();
    if(!emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    sendSetTeleopEnable(false);
    if(axes.x && axes.y && axes.z)
    {
        sendHome(-1); //home all
        return;
    }
    for(int ct=0; ct < MAX_AXES; ct++)
    {
        if(axes.array[ct]) sendHome(ct);
    }
}

Axes LinuxCnc::GetOffset(const unsigned int index)
{
    ThreadLock lock = GetLock();
    Axes ret;
    if(!emcStatus)
    {
        memset(&ret, 0, sizeof(ret));
        return ret;
    }
    EmcPose * pose = NULL;
    switch(index)
    {
    case 0: //tool offset
        pose = &emcStatus->task.toolOffset;
        break;

    case 1: //G5x offset
        pose = &emcStatus->task.g5x_offset;
        break;

    case 2: //G9x offset
        pose = &emcStatus->task.g92_offset;
        break;

    }
    if(pose)
    {
#if LINUXCNC_PRE_JOINTS
        ret.x = pose->tran.x / emcStatus->motion.axis[0].units;
        ret.y = pose->tran.y / emcStatus->motion.axis[1].units;
        ret.z = pose->tran.z / emcStatus->motion.axis[2].units;
        ret.a = pose->a / emcStatus->motion.axis[3].units;
        ret.b = pose->b / emcStatus->motion.axis[4].units;
        ret.c = pose->c / emcStatus->motion.axis[5].units;
#else
        ret.x = pose->tran.x / emcStatus->motion.joint[0].units;
        ret.y = pose->tran.y / emcStatus->motion.joint[1].units;
        ret.z = pose->tran.z / emcStatus->motion.joint[2].units;
        ret.a = pose->a / emcStatus->motion.joint[3].units;
        ret.b = pose->b / emcStatus->motion.joint[4].units;
        ret.c = pose->c / emcStatus->motion.joint[5].units;
#endif
    }else
    {
        memset(&ret, 0, sizeof(ret));
    }
    return ret;
}

vector<int> LinuxCnc::GetGCodes()
{
    ThreadLock lock = GetLock();
    vector<int> ret;
    if(!emcStatus) return ret;
    for(int ct=0; ct < ACTIVE_G_CODES; ct++)
    {
        int code = emcStatus->task.activeGCodes[ct];
        if(code >= 0)
        {
            ret.push_back(code);
        }
    }
    return ret;
}

vector<int> LinuxCnc::GetMCodes()
{
    ThreadLock lock = GetLock();
    vector<int> ret;
    if(!emcStatus) return ret;
    for(int ct=0; ct < ACTIVE_M_CODES; ct++)
    {
        int code = emcStatus->task.activeMCodes[ct];
        if(code >= 0)
        {
            ret.push_back(code);
        }
    }
    return ret;
}


