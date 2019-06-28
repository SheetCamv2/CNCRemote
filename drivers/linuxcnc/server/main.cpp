/****************************************************************
LinuxCNC server main loop
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


#include <iostream>
//#include <cstdint>
#include <time.h>

#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>

#include "getopt.h"
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
#include <syslog.h>


int port = DEFAULT_COMMS_PORT;
string a = PACKAGE_VERSION;
char defaultPath[LINELEN] = {0};
bool isDaemon = false;


struct option longopts[] =
{
    {"help", 0, NULL, 'h'},
    {"port", 1, NULL, 'p'},
    {NULL,0,NULL,0}
};



using namespace std;


LinuxCnc machine;

static void sigQuit(int sig)
{

    machine.DisconnectNml();
    exit(0);
}

/*
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
*/

static void usage(char* pname)
{
    printf("Usage: \n");
    printf("         %s [Options] [-- LinuxCNC_Options]\n"
           "Options:\n"
           "         -h,--help       this help\n"
           "         -p,--port       <port number>  (default=%d)\n"
           "         -d,--daemon     Run as a daemon in the background\n"
//           "LinuxCNC_Options:\n"
//           "         -ini        <inifile>      (default=%s)\n"
           ,pname,port
          );
}

#include "timer.h"


int main(int argc, char * argv[])
{

    int opt;
    while((opt = getopt_long(argc, argv, "h:p:d", longopts, NULL)) != - 1)
    {
        switch(opt)
        {
        case 'h':
            usage(argv[0]);
            exit(1);
        case 'p':
            sscanf(optarg, "%d", &port); break;
            break;
        }
    }

    // process LinuxCNC command line args
    // Note: '--' may be used to separate cmd line args
    //       optind is index of next arg to process
    //       make argv[optind] zeroth arg
    // attach our quit function to SIGINT
    {
        struct sigaction act;
        act.sa_handler = sigQuit;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, NULL);
    }

    if(machine.Bind(port) != CncRemote::errOK)
    {
        printf("Failed to bind port %d. Is another server already running on that port?\n", port);
        return -1;
    }

    while(1)
    {
        printf("Waiting for LinuxCNC\n");
        machine.ConnectLCnc();
        printf("Connected to LinuxCNC\n");

        while(1)
        {
            if(!machine.Poll())
            {
                break;
            }
            SleepMs(1);
        }
        printf("Disconnected from LinuxCNC\n");
        machine.DisconnectNml();
    }
    return 0;
}
