/****************************************************************
LinuxCNC server plugin
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



#include "cncplugin.h"
#include "time.h"
#include "cncclient.h"
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <dirent.h>

string g_serverPath;

enum {EXEC_FAIL = -1, EXEC_OK = -2};

int execute(const char * path, const char * arg, const bool waitExit)
{
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        return -1;
    }
    else if (pid == 0)            // for the child process
    {
        char * const argv[3] ={(char *)path, (char *)arg, NULL};
        if (execvp(path, argv) < 0)
        {
            return -1;
        }
    }

    if(waitExit)
    {
        int status;
        while (wait(&status) != pid);
        if(WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return -1;
    }
    return -2;
}

#define _STRINGIFY(n) #n

EXPORT_CNC uint32_t Start()
{
    CncRemote::Client client;
    CncString tmp;
    if(!client.Connect(0, "localhost",DEFAULT_COMMS_PORT)) return false;

    if(client.Ping(100)) return true; //server is already running
    if(g_serverPath.size() == 0) //no server available
    {
        return false;
    }
    if(execute(g_serverPath.c_str(), NULL, false) != EXEC_OK)
    {
        return false;
    }
    return client.Ping(250);
}

EXPORT_CNC void Stop()
{
}

EXPORT_CNC const char * GetName()
{
#ifdef _DEBUG
    return ("LinuxCNC (debug)");
#else
    return ("LinuxCNC");
#endif
}

EXPORT_CNC const uint32_t ControlExists(const char * pluginDir)
{
    string path = pluginDir;
    path += "linuxcncserver/cncremote.sh";
    int status = execute(path.c_str(), "-c", true);
    if(status == 0)
    {
        g_serverPath = path;
        return true;
    }
    return false;
}

EXPORT_CNC void Quit()
{
}

EXPORT_CNC void Poll()
{

}
