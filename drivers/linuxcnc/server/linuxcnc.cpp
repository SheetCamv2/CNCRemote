#include "linuxcnc.h"


#include "rcs.hh"
#include "posemath.h"		// PM_POSE, TO_RAD
#include "emc.hh"		// EMC NML
#include "canon.hh"		// CANON_UNITS, CANON_UNITS_INCHES,MM,CM
#include "emcglb.h"		// EMC_NMLFILE, TRAJ_MAX_VELOCITY, etc.
#include "emccfg.h"		// DEFAULT_TRAJ_MAX_VELOCITY
//#include "inifile.hh"		// INIFILE
#include "rcs_print.hh"
#include "timer.hh"             // etime()
#include "shcom.hh"             // NML Messaging functions

#include "shcom.cc" //this way we can use the include search path to find shcom.cc
#include "version.h"


LinuxCnc::LinuxCnc()
{
    m_slowCount = 0;
    m_heartbeat = 0;
    m_nextTime = 0;
    m_connected = false;

    m_maxSpeedLin = 4000;
    m_maxSpeedAng = 100;
    halId = -1;
    ZeroJog();

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

    if(emcStatus->size != sizeof(EMC_STAT))
    {
        printf("Wrong LinuxCNC version\n");
        exit(0);
    }

    emcCommandSerialNumber = emcStatus->echo_serial_number;
    m_heartbeat = emcStatus->task.heartbeat;
    m_nextTime = time(NULL) + 1; //check every second
    m_connected = true;

    halId = hal_init("CNCRemote");
    if(halId < 0)
    {
        printf("Failed to connect to HAL\n");
    }else
    {
        for(int ct=0; ct < MAX_AXES; ct++)
        {
            LoadAxis(ct);
        }
        hal_ready(halId);
    }

#define EMC_WAIT_NONE (EMC_WAIT_TYPE) 1
}


bool LinuxCnc::Poll()
{
    if(updateStatus() != 0)
    {
        printf("Disconnected\n");
        Disconnect();
        return false;
    }
    if(emcStatus->motion.traj.maxVelocity < 1e17)
    {
        m_maxSpeedLin = emcStatus->motion.traj.maxVelocity;
        m_maxSpeedAng = emcStatus->motion.traj.maxVelocity;
    }
    Server::Poll();
    if(m_halAxes[0].counts)
    {
        hal_float_t time = (double)m_jogTimer.GetElapsed(true) / 1000000.0; //elapsed time in seconds since last poll
        time *= 10000; //axis is scaled at 1000 counts per unit
        bool found = false;
        double vel;
        for(int ct=0; ct < MAX_AXES; ct++)
        {
            vel = m_jogAxes[ct];
            if(vel != 0)
            {
                if(m_halAxes[ct].counts)
                {
                    *m_halAxes[ct].counts += time * vel;
                }
            }
        }
    }



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
            set_control_on(state);
        }
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
#if MAJOR_VER <= 2 && MINOR_VER <=8
        set_max_feed_lin((m_maxSpeedLin * 60) / emcStatus->motion.axis[0].units);
        set_max_feed_ang((m_maxSpeedAng * 60) / emcStatus->motion.axis[0].units);
#else
        set_max_feed_lin((m_maxSpeedLin * 60) / emcStatus->motion.joint[0].units);
        set_max_feed_ang((m_maxSpeedAng * 60) / emcStatus->motion.joint[0].units);
#endif
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
        {
            static bool prevState = false;
            bool state = emcStatus->task.interpState != EMC_TASK_INTERP_IDLE;
            if(state != prevState)
            {
                ZeroJog();
                prevState = state;
            }
            set_running(state);
        }

        updateError();
        if(error_string[0] != 0)
        {
            set_error_msg(error_string);
            error_string[0] = 0;
        }
        if(operator_text_string[0] != 0)
        {
            set_display_msg(operator_text_string);
            operator_text_string[0] = 0;
        }
        else if(operator_display_string[0] != 0)
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
        CncRemote::Axes& axes = *mutable_offset_work();
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
        CncRemote::BoolAxes& axes = *mutable_homed();
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

inline void LinuxCnc::SendJog(const int axis, const double vel)
{

    if(vel == m_jogAxes[axis]) return;
    if(m_halAxes[axis].counts)
    {
         m_jogAxes[axis] = vel;
        *m_halAxes[axis].enable = true;
        *m_halAxes[axis].velMode = true;
        *m_halAxes[axis].scale = 1/10000.0f;
        return;
    }
    if(m_halAxes[0].counts) return; //using HAL but we don't have this axis in the config



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

    }else
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
    m_jogAxes[axis] = vel;
}

void LinuxCnc::ZeroJog()
{
    memset(m_jogAxes, 0, sizeof(m_jogAxes));
}

int LinuxCnc::SendJogVel(const double x, const double y, const double z, const double a, const double b, const double c)
{

    SendJog(0,x * m_maxSpeedLin);
    SendJog(1,y * m_maxSpeedLin);
    SendJog(2,z * m_maxSpeedLin);
    SendJog(3,a * m_maxSpeedAng);
    SendJog(4,b * m_maxSpeedAng);
    SendJog(5,c * m_maxSpeedAng);
    return 0;

/*
#if MAJOR_VER <= 2 && MINOR_VER <=8

    EMC_TRAJ_SET_TELEOP_VECTOR emc_set_teleop_vector;
    ZERO_EMC_POSE(emc_set_teleop_vector.vector);
    emc_set_teleop_vector.vector.tran.x = x * m_maxSpeedLin;
    emc_set_teleop_vector.vector.tran.y = y * m_maxSpeedLin;
    emc_set_teleop_vector.vector.tran.z = z * m_maxSpeedLin;
    emc_set_teleop_vector.vector.a = a * m_maxSpeedAng;
    emc_set_teleop_vector.vector.b = b * m_maxSpeedAng;
    emc_set_teleop_vector.vector.c = c * m_maxSpeedAng;
#else
    emc_set_teleop_vector.vector.tran.x = x * scale * emcStatus->motion.joint[0].units;
    emc_set_teleop_vector.vector.tran.y = y * scale * emcStatus->motion.joint[1].units;
    emc_set_teleop_vector.vector.tran.z = z * scale * emcStatus->motion.joint[2].units;
    emc_set_teleop_vector.vector.a = a * scale * emcStatus->motion.joint[3].units;
    emc_set_teleop_vector.vector.b = b * scale * emcStatus->motion.joint[4].units;
    emc_set_teleop_vector.vector.c = c * scale * emcStatus->motion.joint[5].units;
#endif


    emcCommandSend(emc_set_teleop_vector);

*/

    return 0;
    /*



    #if MAJOR_VER <= 2 && MINOR_VER <=8

        sendJogCont(axis, val * emcStatus->motion.axis[axis].units);
    #else
        if(emcStatus->motion.joint[axis].jointType == EMC_LINEAR)
            sendJogCont(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units);
        else
            sendJogCont(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units);
    #endif*/
}


void LinuxCnc::SendJogStep(const int axis, const double val)
{
#if MAJOR_VER <= 2 && MINOR_VER <=8

    if(emcStatus->motion.axis[axis].axisType == EMC_AXIS_LINEAR)
        sendJogIncr(axis, val * emcStatus->motion.axis[axis].units, m_maxSpeedLin);
    else
        sendJogIncr(axis, val * emcStatus->motion.axis[axis].units, m_maxSpeedAng);
#else
    if(emcStatus->motion.joint[axis].jointType == EMC_LINEAR)
        sendJogIncr(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units, m_maxSpeedLin);
    else
        sendJogIncr(axis,JOGTELEOP, val * emcStatus->motion.joint[axis].units, m_maxSpeedAng);
#endif
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


hal_data_u * LinuxCnc::FindPin(const char * name, hal_type_t type)
{
    rtapi_mutex_get(&(hal_data->mutex));
    int ptr = hal_data->pin_list_ptr;
    hal_pin_t * pin = NULL;
    while (ptr)
    {
        hal_pin_t *p = (hal_pin_t *)SHMPTR(ptr);
        if(strcmp(p->name, name) == 0)
        {
           pin = p;
           break;
        }
        ptr = p->next_ptr;
    }
    rtapi_mutex_give(&(hal_data->mutex));
    if(pin)
    {
        printf("Found pin %s\n", name);
    }else
    {
        printf("Pin %s not found\n", name);
        return NULL;
    }
    if(pin->type != type)
    {
        printf("Pin %s incorrect type\n", name);
    }

    if(pin->signal == 0)
    {
        return &pin->dummysig;
    }
    hal_sig_t * sig = (hal_sig_t * )SHMPTR(pin->signal);
    return ((hal_data_u *)SHMPTR(sig->data_ptr));
}

void LinuxCnc::LoadAxis(const int index)
{
    JOGAXIS& axis = m_halAxes[index];
    char buf[256];
/*

                int o = m_halAxes[ct].counts->type;
                hal_sig_t *sig = (hal_sig_t *)SHMPTR(m_halAxes[ct].counts->signal);
                hal_float_t * ptr = (hal_float_t *)SHMPTR(sig->data_ptr);*/
    sprintf(buf,"axis.%i.jog-counts", index);
    hal_data_u * dat = FindPin(buf, HAL_S32);
    bool err = false;
    if(dat) //sanity check
    {
        axis.counts = &dat->s;
    }else
    {
        err = true;
    }

    sprintf(buf,"axis.%i.jog-enable", index);
    dat = FindPin(buf, HAL_BIT);
    if(dat)
    {
        axis.enable = &dat->b;//(char *)SHMPTR(sig->data_ptr);
    }else
    {
        err = true;
    }

    sprintf(buf,"axis.%i.jog-scale", index);
    dat = FindPin(buf, HAL_FLOAT);
    if(dat) //sanity check
    {
        axis.scale = &dat->f;//(hal_float_t *)SHMPTR(sig->data_ptr);
    }else
    {
        err = true;
    }

    sprintf(buf,"axis.%i.jog-vel-mode", index);
    dat = FindPin(buf, HAL_BIT);
    if(dat)
    {
        axis.velMode = &dat->b;//(char *)SHMPTR(sig->data_ptr);
    }else
    {
        err = true;
    }
    if(err)
    {
        memset(&axis, 0, sizeof(JOGAXIS));
    }
}



