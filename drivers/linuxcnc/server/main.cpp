#include <iostream>
#include <cstdint>
#include <time.h>

#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>



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
#include "getopt.h"

#include "cnccomms.h"
#include "millisleep.h"


int port = 5010;

struct option longopts[] = {
  {"help", 0, NULL, 'h'},
  {"port", 1, NULL, 'p'},
  {"path", 1, NULL, 'd'},
  {NULL,0,NULL,0}
};



using namespace std;


static void Disconnect()
{
//    EMC_NULL emc_null_msg;

    if (emcStatusBuffer != 0) // wait until current message has been received
    {
        emcCommandWaitReceived();
    }

	delete emcErrorBuffer;
	emcErrorBuffer = 0;
	delete emcStatusBuffer;
	emcStatusBuffer = 0;
	emcStatus = 0;
	delete emcCommandBuffer;
	emcCommandBuffer = 0;
}


static void sigQuit(int sig)
{
    Disconnect();
    exit(0);
}


static void initMain()
{
    emcWaitType = EMC_WAIT_RECEIVED;
    emcCommandSerialNumber = 0;
    emcTimeout = 0.0;
    emcUpdateType = EMC_UPDATE_AUTO;
    linearUnitConversion = LINEAR_UNITS_AUTO;
    angularUnitConversion = ANGULAR_UNITS_AUTO;
    emcCommandBuffer = 0;
    emcStatusBuffer = 0;
    emcStatus = 0;

    emcErrorBuffer = 0;
    error_string[LINELEN-1] = 0;
    operator_text_string[LINELEN-1] = 0;
    operator_display_string[LINELEN-1] = 0;
    programStartLine = 0;
}

static void usage(char* pname) {
    printf("Usage: \n");
    printf("         %s [Options] [-- LinuxCNC_Options]\n"
           "Options:\n"
           "         --help       this help\n"
           "         --port       <port number>  (default=%d)\n"
           "         --path       <path>         (default=%s)\n"
           "LinuxCNC_Options:\n"
           "          -ini        <inifile>      (default=%s)\n"
          ,pname,port,defaultPath,emc_inifile
          );
}


class LinuxCncMachine : public CncComms
{
public:
    LinuxCncMachine(const string ipAddress, const string statePort, const string cmdPort) : CncComms(ipAddress,statePort,cmdPort, true)
    {
        m_slowCount = 0;
        m_heartbeat = 0;
        m_nextTime = 0;
    }

    void Connect()
    {
        while(tryNml(0.5,0.5) !=0)
        {
            set_machine_connected(false);
            Send();
            CncComms::Poll();
        }
        // init NML
        // get current serial number, and save it for restoring when we quit
        // so as not to interfere with real operator interface
        updateStatus();
        emcCommandSerialNumber = emcStatus->echo_serial_number;
        m_heartbeat = emcStatus->task.heartbeat;
        m_nextTime = time(NULL) + 1; //check every second
    }

    bool Poll()
    {
       if(updateStatus() != 0)
       {
            Disconnect();
            return false;
       }

       Clear();
       CncAxes& axes = *mutable_abs_pos();
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
                set_currentline(emcStatus->task.motionLine);
                break;

            case 8:
                set_lin_unit_scale(emcStatus->motion.traj.linearUnits);
                break;

            case 9:
            {
                CncAxes& axes = *mutable_offset_fixture();
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
                CncAxes& axes = *mutable_offset_work();
                axes.set_x(emcStatus->task.g92_offset.tran.x);
                axes.set_y(emcStatus->task.g92_offset.tran.y);
                axes.set_z(emcStatus->task.g92_offset.tran.z);
                axes.set_a(emcStatus->task.g92_offset.a);
                axes.set_b(emcStatus->task.g92_offset.b);
                axes.set_c(emcStatus->task.g92_offset.c);
            }
                break;

            default:
                if(time(NULL) > m_nextTime)
                {
                    if(m_heartbeat != emcStatus->task.heartbeat)
                    {
                        m_heartbeat = emcStatus->task.heartbeat;
                    }else
                    {
                        return false;
                    }
                    m_nextTime = time(NULL) + 1; //check every second
                }
                m_slowCount = 0;
       }
       Send();
       CncComms::Poll();
       return true;
    }

private:

    virtual void ExecuteCommand(const CncCmdBuf& cmd)
    {
        switch(cmd.type())
        {
        case cmdDRIVESON:
            if(cmd.state())
            {
                sendEstopReset();
                sendMachineOn();
            }else
            {
                sendEstop();
            }
            break;

        case cmdJOGVEL:
            SetMode(EMC_TASK_MODE_MANUAL);
            {
                const CncAxes& axes = cmd.axes();
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
            }else
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
        }
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

    int m_slowCount;
    uint32_t m_heartbeat;
    time_t m_nextTime;
};



int main(int argc, char * argv[])
{
    int opt;

    initMain();
    // process local command line args
    while((opt = getopt_long(argc, argv, "he:p:d:", longopts, NULL)) != - 1) {
      switch(opt) {
        case 'h': usage(argv[0]); exit(1);
        case 'p': sscanf(optarg, "%d", &port); break;
        case 'd': strncpy(defaultPath, optarg, strlen(optarg) + 1); break;
        }
      }

    // process LinuxCNC command line args
    // Note: '--' may be used to separate cmd line args
    //       optind is index of next arg to process
    //       make argv[optind] zeroth arg
    argc = argc - optind + 1;
    argv = argv + optind - 1;
    if (emcGetArgs(argc, argv) != 0) {
	rcs_print_error("error in argument list\n");
	exit(1);
    }

    // attach our quit function to SIGINT
    {
        struct sigaction act;
        act.sa_handler = sigQuit;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, NULL);
    }


    char port1[20];
    char port2[20];
    sprintf(port1,"%d",port);
    sprintf(port2,"%d",port + 1);
    LinuxCncMachine machine("*", port1, port2); //bind to all adapters

    while(1)
    {
        // get configuration information
        iniLoad(emc_inifile);
        machine.Connect();
        while(machine.Poll())
        {
           sleep_ms(2);
        }
        Disconnect();
    }
    return 0;
}
