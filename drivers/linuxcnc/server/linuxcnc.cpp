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
    m_lastMessage = NULL;
    m_emcCommandBuffer = NULL;
    m_emcStatusBuffer = NULL;
    m_emcStatus = NULL;
    m_emcErrorBuffer = NULL;
    m_linearUnitConversion = LINEAR_UNITS_MM;


    g_maxSpeedLin = 4000;
    g_maxSpeedAng = 100;
    halId = -1;
    memset(g_jogAxes, 0, sizeof(g_jogAxes));

}

void LinuxCnc::DisconnectNml()
{
    delete m_emcCommandBuffer;
    delete m_emcStatusBuffer;
    delete m_emcErrorBuffer;
    m_emcCommandBuffer = NULL;
    m_emcStatusBuffer = NULL;
    m_emcStatus = NULL;
    m_emcErrorBuffer = NULL;
}

bool LinuxCnc::ConnectNml()
{

    // try to connect to EMC cmd
    if (m_emcCommandBuffer == NULL)
    {
        m_emcCommandBuffer =
            new RCS_CMD_CHANNEL(emcFormat, "emcCommand", "xemc", emc_nmlfile);
        if (!m_emcCommandBuffer->valid())
        {
            DisconnectNml();
            return false;
        }
    }
    // try to connect to EMC status
    if (m_emcStatusBuffer == NULL)
    {
        m_emcStatusBuffer =
            new RCS_STAT_CHANNEL(emcFormat, "emcStatus", "xemc", emc_nmlfile);
        if (!m_emcStatusBuffer->valid()
                || EMC_STAT_TYPE != m_emcStatusBuffer->peek())
        {
            DisconnectNml();
            return false;
        }
        else
        {
            m_emcStatus = (EMC_STAT *) m_emcStatusBuffer->get_address();
        }
    }

    if (m_emcErrorBuffer == NULL)
    {
        m_emcErrorBuffer =
            new NML(nmlErrorFormat, "emcError", "xemc", emc_nmlfile);
        if (!m_emcErrorBuffer->valid())
        {
            DisconnectNml();
            return false;
        }
    }
    return OK();
}


bool LinuxCnc::OK()
{
    return (m_emcStatus != NULL &&
            m_emcStatusBuffer != NULL &&
            m_emcStatusBuffer->valid() &&
            m_emcErrorBuffer != NULL &&
            m_emcErrorBuffer->valid());
}

int LinuxCnc::IniLoad(const char *filename)
{
    IniFile inifile;
    const char *inistring;
//    char displayString[LINELEN] = "";

    // open it
    if (inifile.Open(filename) == false)
    {
        return -1;
    }

    if (NULL != (inistring = inifile.Find("DEBUG", "EMC")))
    {
        // copy to global
        if (1 != sscanf(inistring, "%i", &emc_debug))
        {
            emc_debug = 0;
        }
    }
    else
    {
        // not found, use default
        emc_debug = 0;
    }

    if (NULL != (inistring = inifile.Find("NML_FILE", "EMC")))
    {
        // copy to global
        strcpy(emc_nmlfile, inistring);
    }
    else
    {
        // not found, use default
    }

    /*    for (t = 0; t < EMCMOT_MAX_JOINTS; t++) {
    	jogPol[t] = 1;		// set to default
    	sprintf(displayString, "JOINT_%d", t);
    	if (NULL != (inistring =
    		     inifile.Find("JOGGING_POLARITY", displayString)) &&
    	    1 == sscanf(inistring, "%d", &i) && i == 0) {
    	    // it read as 0, so override default
    	    jogPol[t] = 0;
    	}
        }*/

    if (NULL != (inistring = inifile.Find("LINEAR_UNITS", "DISPLAY")))
    {
        if (!strcmp(inistring, "AUTO"))
        {
            m_linearUnitConversion = LINEAR_UNITS_AUTO;
        }
        else if (!strcmp(inistring, "INCH"))
        {
            m_linearUnitConversion = LINEAR_UNITS_INCH;
        }
        else if (!strcmp(inistring, "MM"))
        {
            m_linearUnitConversion = LINEAR_UNITS_MM;
        }
        else if (!strcmp(inistring, "CM"))
        {
            m_linearUnitConversion = LINEAR_UNITS_CM;
        }
    }
    else
    {
        // not found, leave default alone
    }

    /*    if (NULL != (inistring = inifile.Find("ANGULAR_UNITS", "DISPLAY")))
        {
            if (!strcmp(inistring, "AUTO"))
            {
                angularUnitConversion = ANGULAR_UNITS_AUTO;
            }
            else if (!strcmp(inistring, "DEG"))
            {
                angularUnitConversion = ANGULAR_UNITS_DEG;
            }
            else if (!strcmp(inistring, "RAD"))
            {
                angularUnitConversion = ANGULAR_UNITS_RAD;
            }
            else if (!strcmp(inistring, "GRAD"))
            {
                angularUnitConversion = ANGULAR_UNITS_GRAD;
            }
        }
        else
        {
            // not found, leave default alone
        }*/

    // close it
    inifile.Close();

    return 0;
}


bool LinuxCnc::ConnectLCnc()
{
    /*    linear::mutex& mutex = GetMutex();
        mutex.unlock();
        {
            LockedState s = GetState();
            s->machineState = mcOFFLINE;
        }
        mutex.lock();*/

    set_rcs_print_destination(RCS_PRINT_TO_NULL);
    while(!ConnectNml())
    {
        esleep(0.5);
    }

    NMLTYPE type = m_emcStatusBuffer->peek();
    if(type == -1) return false; //error*/

    if(m_emcStatus->size != sizeof(EMC_STAT))
    {
        printf("Wrong LinuxCNC version\n");
        exit(0);
    }

    m_heartbeat = m_emcStatus->task.heartbeat;
    m_nextTime = time(NULL) + 1; //check every second

    if(m_emcStatus->motion.traj.maxVelocity < 1e17)
    {
        g_maxSpeedLin = m_emcStatus->motion.traj.maxVelocity;
        g_maxSpeedAng = m_emcStatus->motion.traj.maxVelocity;
    }

    return OK();
#define EMC_WAIT_NONE (EMC_WAIT_TYPE) 1
}


bool LinuxCnc::Poll()
{
    ThreadLock lock = GetLock();
    if(!OK()) return false;
//    type=emcErrorBuffer->read();
    NMLTYPE type=m_emcErrorBuffer->peek();
    if(type == -1)
    {
//        return false; //error
        return true; //error
    }
    if(type == 0) return true; //no data
    void * ptr = m_emcErrorBuffer->get_address();
    if(ptr == m_lastMessage) return true; //Message has not changed
    m_lastMessage = ptr;

    char buf[LINELEN + 1];
    switch(type)
    {
    case -1: //error
        return true;
//        return false;

    case EMC_OPERATOR_ERROR_TYPE:
        strncpy(buf, ((EMC_OPERATOR_ERROR *) ptr)-> error, LINELEN);
        buf[LINELEN] = 0;
        LogError(buf);
        break;

    case EMC_OPERATOR_TEXT_TYPE:
        strncpy(buf, ((EMC_OPERATOR_TEXT *) ptr)-> text, LINELEN);
        buf[LINELEN] = 0;
        LogMessage(buf);
        break;

    case EMC_OPERATOR_DISPLAY_TYPE:
        strncpy(buf, ((EMC_OPERATOR_DISPLAY *) ptr)-> display, LINELEN);
        buf[LINELEN] = 0;
        LogMessage(buf);
        break;

    case NML_ERROR_TYPE:
        strncpy(buf, ((NML_ERROR *) ptr)-> error, LINELEN);
        buf[LINELEN] = 0;
        LogError(buf);
        break;

    case NML_TEXT_TYPE:
        strncpy(buf, ((NML_TEXT *) ptr)-> text, LINELEN);
        buf[LINELEN] = 0;
        LogMessage(buf);
        break;

    case NML_DISPLAY_TYPE:
        strncpy(buf, ((NML_DISPLAY *) ptr)-> display, LINELEN);
        buf[LINELEN] = 0;
        LogMessage(buf);
        break;

    default:
        LogError("Unknown error");
        break;
    }

    return true;
}


void LinuxCnc::UpdateState(State& state)
{
    if(m_emcStatus == NULL)
    {
        state.machineState = mcOFFLINE;
        return;
    }


    if(!OK() || m_emcStatusBuffer->peek() == -1)
    {
//        printf("Disconnected\n");
//        state.machineState = mcOFFLINE;
//        DisconnectNml();
        return;
    }


    if(time(NULL) > m_nextTime)
    {
        if(m_heartbeat != m_emcStatus->task.heartbeat)
        {
            m_heartbeat = m_emcStatus->task.heartbeat;
        }
        else
        {
            printf("Client disconnected\n");
            state.machineState = mcOFFLINE;
            DisconnectNml();
            return;
        }
        m_nextTime = time(NULL) + 1; //check every second
    }


    {
        CncRemote::Axes& axes = state.machinePos;
#if LINUXCNC_PRE_JOINTS
        axes.x = m_emcStatus->motion.traj.actualPosition.tran.x / m_emcStatus->motion.axis[0].units;
        axes.y = m_emcStatus->motion.traj.actualPosition.tran.y / m_emcStatus->motion.axis[1].units;
        axes.z = m_emcStatus->motion.traj.actualPosition.tran.z / m_emcStatus->motion.axis[2].units;
        axes.a = m_emcStatus->motion.traj.actualPosition.a / m_emcStatus->motion.axis[3].units;
        axes.b = m_emcStatus->motion.traj.actualPosition.b / m_emcStatus->motion.axis[4].units;
        axes.c = m_emcStatus->motion.traj.actualPosition.c / m_emcStatus->motion.axis[5].units;
#else
        axes.x = m_emcStatus->motion.traj.actualPosition.tran.x / m_emcStatus->motion.joint[0].units;
        axes.y = m_emcStatus->motion.traj.actualPosition.tran.y / m_emcStatus->motion.joint[1].units;
        axes.z = m_emcStatus->motion.traj.actualPosition.tran.z / m_emcStatus->motion.joint[2].units;
        axes.a = m_emcStatus->motion.traj.actualPosition.a / m_emcStatus->motion.joint[3].units;
        axes.b = m_emcStatus->motion.traj.actualPosition.b / m_emcStatus->motion.joint[4].units;
        axes.c = m_emcStatus->motion.traj.actualPosition.c / m_emcStatus->motion.joint[5].units;
#endif
    }

    {
        CncRemote::Axes& axes = state.position;


#if LINUXCNC_PRE_JOINTS
        axes.x = (m_emcStatus->motion.traj.actualPosition.tran.x - m_emcStatus->task.g5x_offset.tran.x - m_emcStatus->task.g92_offset.tran.x) / m_emcStatus->motion.axis[0].units;
        axes.y = (m_emcStatus->motion.traj.actualPosition.tran.y - m_emcStatus->task.g5x_offset.tran.y - m_emcStatus->task.g92_offset.tran.y) / m_emcStatus->motion.axis[1].units;
        axes.z = (m_emcStatus->motion.traj.actualPosition.tran.z - m_emcStatus->task.g5x_offset.tran.z - m_emcStatus->task.g92_offset.tran.z) / m_emcStatus->motion.axis[2].units;
        axes.a = (m_emcStatus->motion.traj.actualPosition.a      - m_emcStatus->task.g5x_offset.a      - m_emcStatus->task.g92_offset.a)      / m_emcStatus->motion.axis[3].units;
        axes.b = (m_emcStatus->motion.traj.actualPosition.b      - m_emcStatus->task.g5x_offset.b      - m_emcStatus->task.g92_offset.b)      / m_emcStatus->motion.axis[4].units;
        axes.c = (m_emcStatus->motion.traj.actualPosition.c      - m_emcStatus->task.g5x_offset.c      - m_emcStatus->task.g92_offset.c)      / m_emcStatus->motion.axis[5].units;
#else
        axes.x = (m_emcStatus->motion.traj.actualPosition.tran.x - m_emcStatus->task.g5x_offset.tran.x - m_emcStatus->task.g92_offset.tran.x) / m_emcStatus->motion.joint[0].units;
        axes.y = (m_emcStatus->motion.traj.actualPosition.tran.y - m_emcStatus->task.g5x_offset.tran.y - m_emcStatus->task.g92_offset.tran.y) / m_emcStatus->motion.joint[1].units;
        axes.z = (m_emcStatus->motion.traj.actualPosition.tran.z - m_emcStatus->task.g5x_offset.tran.z - m_emcStatus->task.g92_offset.tran.z) / m_emcStatus->motion.joint[2].units;
        axes.a = (m_emcStatus->motion.traj.actualPosition.a      - m_emcStatus->task.g5x_offset.a      - m_emcStatus->task.g92_offset.a)      / m_emcStatus->motion.joint[3].units;
        axes.b = (m_emcStatus->motion.traj.actualPosition.b      - m_emcStatus->task.g5x_offset.b      - m_emcStatus->task.g92_offset.b)      / m_emcStatus->motion.joint[4].units;
        axes.c = (m_emcStatus->motion.traj.actualPosition.c      - m_emcStatus->task.g5x_offset.c      - m_emcStatus->task.g92_offset.c)      / m_emcStatus->motion.joint[5].units;
#endif
    }




    if(m_emcStatus->task.state == EMC_TASK_STATE_ON)
    {

        if(m_emcStatus->motion.traj.current_vel == 0)
        {
            state.machineState = mcIDLE;
        }
        else
        {
            state.machineState = mcMOVING;
        }
        if(m_emcStatus->task.interpState > EMC_TASK_INTERP_IDLE)
        {
            if(m_emcStatus->task.mode == EMC_TASK_MODE_MDI)
            {
                state.machineState = mcMDI;
            }
            else
            {
                state.machineState = mcRUNNING;
            }
        }

    }
    else
    {
        state.machineState = mcOFF;
    }

    if(state.machineState != mcIDLE) ZeroJog();

#if LINUXCNC_MULTI_SPINDLE
    switch(m_emcStatus->motion.spindle[0].enabled)
#else
    switch(m_emcStatus->motion.spindle.enabled)
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
    state.spindleCmd = m_emcStatus->motion.spindle[0].speed;
    state.spindleActual = m_emcStatus->motion.spindle[0].speed; //FIXME: Should be actual spindle speed
#else
    state.spindleCmd = m_emcStatus->motion.spindle.speed;
    state.spindleActual = m_emcStatus->motion.spindle.speed; //FIXME: Should be actual spindle speed
#endif

    state.feedHold = m_emcStatus->task.task_paused;
#if LINUXCNC_PRE_JOINTS
    state.maxFeedLin = ((g_maxSpeedLin * 60) / m_emcStatus->motion.traj.linearUnits);
    state.maxFeedAng = ((g_maxSpeedAng * 60) / m_emcStatus->motion.traj.angularUnits);
#else
    state.maxFeedLin = ((g_maxSpeedLin * 60) / m_emcStatus->motion.traj.linearUnits);
    state.maxFeedAng = ((g_maxSpeedAng * 60) / m_emcStatus->motion.traj.angularUnits);
#endif
    state.optionalStop = m_emcStatus->task.optional_stop_state;
    state.blockDelete = m_emcStatus->task.block_delete_state;
    state.mist = m_emcStatus->io.coolant.mist;
    state.flood = m_emcStatus->io.coolant.flood;

    state.currentLine = m_emcStatus->task.motionLine;

    {
        CncRemote::BoolAxes& axes = state.homed;
#if LINUXCNC_PRE_JOINTS
        axes.x = m_emcStatus->motion.axis[0].homed;
        axes.y = m_emcStatus->motion.axis[1].homed;
        axes.z = m_emcStatus->motion.axis[2].homed;
        axes.a = m_emcStatus->motion.axis[3].homed;
        axes.b = m_emcStatus->motion.axis[4].homed;
        axes.c = m_emcStatus->motion.axis[5].homed;
#else
        axes.x = m_emcStatus->motion.joint[0].homed;
        axes.y = m_emcStatus->motion.joint[1].homed;
        axes.z = m_emcStatus->motion.joint[2].homed;
        axes.a = m_emcStatus->motion.joint[3].homed;
        axes.b = m_emcStatus->motion.joint[4].homed;
        axes.c = m_emcStatus->motion.joint[5].homed;
#endif
    }
    switch (m_linearUnitConversion)
    {
    case LINEAR_UNITS_MM:
        state.gcodeUnits = 1;
        break;
    case LINEAR_UNITS_INCH:
        state.gcodeUnits =  INCH_PER_MM;
        break;
    case LINEAR_UNITS_CM:
        state.gcodeUnits = CM_PER_MM;
        break;
    case LINEAR_UNITS_AUTO:
        switch (m_emcStatus->task.programUnits)
        {
        case CANON_UNITS_MM:
            state.gcodeUnits = 1;
            break;
        case CANON_UNITS_INCHES:
            state.gcodeUnits = INCH_PER_MM;
            break;
        case CANON_UNITS_CM:
            state.gcodeUnits = CM_PER_MM;
            break;
        }
        break;

    case LINEAR_UNITS_CUSTOM:
        state.gcodeUnits = 1;
        break;
    }



}


void LinuxCnc::ZeroJog()
{
    /*    for(int ct=0; ct < MAX_AXES; ct++)
        {
            LinuxConnection::SendJog(ct, 0);
        }*/
}

bool LinuxCnc::CommandSend(RCS_CMD_MSG & cmd)
{
    if(!m_emcStatus) return false;
    // write command
    if (m_emcCommandBuffer->write(&cmd))
    {
        return false;
    }
    int serial = cmd.serial_number;
    for (int ct=0; ct < 1000; ct++)
    {
        if(m_emcStatusBuffer->peek() < 0) return false;
        int serial_diff = m_emcStatus->echo_serial_number - serial;
        if (serial_diff >= 0)
        {
            return 0;
        }
        esleep(0.001);
    }

    return false;
}

bool LinuxCnc::SetMode(const EMC_TASK_MODE_ENUM mode)
{

    if(m_emcStatus->task.mode == mode) return true;

    EMC_TASK_SET_MODE mode_msg;
    mode_msg.mode = mode;
    return CommandSend(mode_msg);
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
        CommandSend(emc_axis_jog_msg);
#else
        EMC_JOG_CONT emc_jog_cont_msg;
        emc_jog_cont_msg.jjogmode = JOGTELEOP;
        if(m_emcStatus->motion.traj.mode == EMC_TRAJ_MODE_FREE) emc_jog_cont_msg.jjogmode = JOGJOINT;
        emc_jog_cont_msg.joint_or_axis = axis;
        emc_jog_cont_msg.vel = vel;
        CommandSend(emc_jog_cont_msg);
#endif

    }
    else
    {
#if LINUXCNC_PRE_JOINTS

        EMC_AXIS_ABORT emc_axis_abort_msg;
        emc_axis_abort_msg.axis = axis;
        CommandSend(emc_axis_abort_msg);
#else
        EMC_JOG_STOP emc_jog_stop_msg;
        emc_jog_stop_msg.jjogmode = JOGTELEOP;
        if(m_emcStatus->motion.traj.mode == EMC_TRAJ_MODE_FREE) emc_jog_stop_msg.jjogmode = JOGJOINT;
        emc_jog_stop_msg.joint_or_axis = axis;
        CommandSend(emc_jog_stop_msg);
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
    EMC_JOG_INCR emc_jog_incr_msg;

#if LINUXCNC_PRE_JOINTS
    if(m_emcStatus->motion.axis[axis].axisType == EMC_AXIS_LINEAR)
        emc_jog_incr_msg.vel = g_maxSpeedLin / 60.0;
    else
        emc_jog_incr_msg.vel = g_maxSpeedAng / 60.0;
    emc_jog_incr_msg.axis = axis;
#else
    if(m_emcStatus->motion.joint[axis].jointType == EMC_LINEAR)
        emc_jog_incr_msg.vel = g_maxSpeedLin / 60.0;
    else
        emc_jog_incr_msg.vel = g_maxSpeedAng / 60.0;
    emc_jog_incr_msg.jjogmode = JOGTELEOP;
    emc_jog_incr_msg.joint_or_axis = axis;
#endif
    emc_jog_incr_msg.incr = val;

    CommandSend(emc_jog_incr_msg);

}




void LinuxCnc::DrivesOn(const bool state)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;

    EMC_TASK_SET_STATE state_msg;

    if(state)
    {
        state_msg.state = EMC_TASK_STATE_ESTOP_RESET;
        CommandSend(state_msg);
        state_msg.state = EMC_TASK_STATE_ON;
        CommandSend(state_msg);
    }
    else
    {
        state_msg.state = EMC_TASK_STATE_OFF;
        CommandSend(state_msg);
    }
}

void LinuxCnc::JogVel(const Axes velocities)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    SendJogVel(velocities.x, velocities.y, velocities.z, velocities.a, velocities.b, velocities.c);
}

void LinuxCnc::JogStep(const Axes distance, const double speed)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    EMC_TRAJ_SET_TELEOP_ENABLE emc_set_teleop_enable_msg;
    emc_set_teleop_enable_msg.enable = true;
    if(!CommandSend(emc_set_teleop_enable_msg)) return;

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
    if(!m_emcStatus) return false;
    SetMode(EMC_TASK_MODE_MDI);

    EMC_TASK_PLAN_EXECUTE emc_task_plan_execute_msg;
    strncpy(emc_task_plan_execute_msg.command, line.c_str(), LINELEN);
    emc_task_plan_execute_msg.command[LINELEN-1] = 0;
    return CommandSend(emc_task_plan_execute_msg);
}

void LinuxCnc::SpindleOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;

    EMC_TRAJ_SET_SPINDLE_SCALE emc_traj_set_spindle_scale_msg;

    double override = percent;
    if (override < 0.0)
    {
        override = 0.0;
    }

#if LINUXCNC_MULTI_SPINDLE
    emc_traj_set_spindle_scale_msg.spindle = 0;
#endif

    emc_traj_set_spindle_scale_msg.scale = override;
    CommandSend(emc_traj_set_spindle_scale_msg);
}

void LinuxCnc::FeedOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;

    double override = percent;
    if (override < 0.0)
    {
        override = 0.0;
    }

    EMC_TRAJ_SET_SCALE emc_traj_set_scale_msg;
    emc_traj_set_scale_msg.scale = override;
    CommandSend(emc_traj_set_scale_msg);
}

void LinuxCnc::RapidOverride(const double percent)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    double override = percent;
    if (override < 0.0)
    {
        override = 0.0;
    }

    EMC_TRAJ_SET_RAPID_SCALE emc_traj_set_scale_msg;
    emc_traj_set_scale_msg.scale = override;
    CommandSend(emc_traj_set_scale_msg);
}

bool LinuxCnc::LoadFile(const string fileName)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return false;

    EMC_TASK_PLAN_OPEN emc_task_plan_open_msg;
    strncpy(emc_task_plan_open_msg.file, fileName.c_str(), LINELEN);
    emc_task_plan_open_msg.file[LINELEN - 1] = 0;
    return CommandSend(emc_task_plan_open_msg);
}

bool LinuxCnc::CloseFile()
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return false;

    EMC_TASK_PLAN_OPEN emc_task_plan_open_msg;
    emc_task_plan_open_msg.file[0] = 0;
    return CommandSend(emc_task_plan_open_msg);
}

void LinuxCnc::CycleStart()
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    SetMode(EMC_TASK_MODE_AUTO);
    EMC_TASK_PLAN_RUN emc_task_plan_run_msg;
    emc_task_plan_run_msg.line = 0;
    CommandSend(emc_task_plan_run_msg);
}

void LinuxCnc::CycleStop()
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    EMC_TASK_ABORT task_abort_msg;
    CommandSend(task_abort_msg);
}

void LinuxCnc::FeedHold(const bool state)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    if(state)
    {
        EMC_TASK_PLAN_PAUSE emc_task_plan_pause_msg;
        CommandSend(emc_task_plan_pause_msg);
    }
    else
    {
        EMC_TASK_PLAN_RESUME emc_task_plan_resume_msg;
        CommandSend(emc_task_plan_resume_msg);
    }
}

void LinuxCnc::BlockDelete(const bool state)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    EMC_TASK_PLAN_SET_BLOCK_DELETE emc_task_plan_set_block_delete_msg;
    emc_task_plan_set_block_delete_msg.state = state;
    CommandSend(emc_task_plan_set_block_delete_msg);
}

void LinuxCnc::SingleStep(const bool state)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    EMC_TASK_PLAN_STEP emc_task_plan_step_msg;
    CommandSend(emc_task_plan_step_msg);
}

void LinuxCnc::OptionalStop(const bool state)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;

    EMC_TASK_PLAN_SET_OPTIONAL_STOP emc_task_plan_set_optional_stop_msg;
    emc_task_plan_set_optional_stop_msg.state = state;
    CommandSend(emc_task_plan_set_optional_stop_msg);
}

void LinuxCnc::Home(const BoolAxes axes)
{
    ThreadLock lock = GetLock();
    if(!m_emcStatus) return;
    SetMode(EMC_TASK_MODE_MANUAL);
    EMC_TRAJ_SET_TELEOP_ENABLE emc_set_teleop_enable_msg;
    emc_set_teleop_enable_msg.enable = true;
    CommandSend(emc_set_teleop_enable_msg);

    EMC_JOINT_HOME emc_joint_home_msg;

    if(axes.x && axes.y && axes.z)
    {
        emc_joint_home_msg.joint = -1;
        CommandSend(emc_joint_home_msg);
        return;
    }
    for(int ct=0; ct < MAX_AXES; ct++)
    {
        emc_joint_home_msg.joint = ct;
        CommandSend(emc_joint_home_msg);
    }
}

Axes LinuxCnc::GetOffset(const unsigned int index)
{
    ThreadLock lock = GetLock();
    Axes ret;
    if(!m_emcStatus)
    {
        memset(&ret, 0, sizeof(ret));
        return ret;
    }
    EmcPose * pose = NULL;
    switch(index)
    {
    case 0: //tool offset
        pose = &m_emcStatus->task.toolOffset;
        break;

    case 1: //G5x offset
        pose = &m_emcStatus->task.g5x_offset;
        break;

    case 2: //G9x offset
        pose = &m_emcStatus->task.g92_offset;
        break;

    }
    if(pose)
    {
#if LINUXCNC_PRE_JOINTS
        ret.x = pose->tran.x / m_emcStatus->motion.axis[0].units;
        ret.y = pose->tran.y / m_emcStatus->motion.axis[1].units;
        ret.z = pose->tran.z / m_emcStatus->motion.axis[2].units;
        ret.a = pose->a / m_emcStatus->motion.axis[3].units;
        ret.b = pose->b / m_emcStatus->motion.axis[4].units;
        ret.c = pose->c / m_emcStatus->motion.axis[5].units;
#else
        ret.x = pose->tran.x / m_emcStatus->motion.joint[0].units;
        ret.y = pose->tran.y / m_emcStatus->motion.joint[1].units;
        ret.z = pose->tran.z / m_emcStatus->motion.joint[2].units;
        ret.a = pose->a / m_emcStatus->motion.joint[3].units;
        ret.b = pose->b / m_emcStatus->motion.joint[4].units;
        ret.c = pose->c / m_emcStatus->motion.joint[5].units;
#endif
    }
    else
    {
        memset(&ret, 0, sizeof(ret));
    }
    return ret;
}

vector<int> LinuxCnc::GetGCodes()
{
    ThreadLock lock = GetLock();
    vector<int> ret;
    if(!m_emcStatus) return ret;
    for(int ct=0; ct < ACTIVE_G_CODES; ct++)
    {
        int code = m_emcStatus->task.activeGCodes[ct];
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
    if(!m_emcStatus) return ret;
    for(int ct=0; ct < ACTIVE_M_CODES; ct++)
    {
        int code = m_emcStatus->task.activeMCodes[ct];
        if(code >= 0)
        {
            ret.push_back(code);
        }
    }
    return ret;
}


