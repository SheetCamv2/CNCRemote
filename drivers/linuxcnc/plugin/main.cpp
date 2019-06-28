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

CNCLOGFUNC LogFunc = NULL;

CncChar msgBuf[256];

FILE * exec = NULL;
int execPid = -1;
int execRet = -1;

#ifdef __VISUALC__
#define Log(fmt,...) {sprintf(msgBuf, fmt, __VA_ARGS__ ); LogFunc(msgBuf);}
#else
#define Log(fmt,args...) {sprintf(msgBuf, fmt, ## args ); LogFunc(msgBuf);}
#endif



#define READ   0
#define WRITE  1
FILE * popen2(string command, string type, int & pid)
{
    pid_t child_pid;
    int fd[2];
    pipe(fd);

    if((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    /* child process */
    if (child_pid == 0)
    {
        if (type == "r")
        {
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
        }
        else
        {
            close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0);   //Redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
        execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
        exit(0);
    }
    else
    {
        if (type == "r")
        {
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else
        {
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (type == "r")
    {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int pclose2(FILE * fp, pid_t pid)
{
    int stat;

    fclose(fp);
    while (waitpid(pid, &stat, 0) == -1)
    {
        if (errno != EINTR)
        {
            stat = -1;
            break;
        }
    }

    return stat;
}



bool ExecPoll()
{
    char buf[256];
    if(!exec) return false;
    if(feof(exec))
    {
        execRet = pclose2(exec, execPid);
        exec = NULL;
        return false;
    }
    if(fgets(buf, 255, exec) == NULL) return false;
    int l = strlen(buf) - 1;
    while (l > 0 && buf[l] <= 32)
    {
        buf[l] = 0;
        l--;
    }
    LogFunc(buf);
    return true;
}

bool ExecStart(const char* cmd)
{
    execRet = -1;
    exec = popen2(cmd, "r", execPid);
    if(!exec)
    {
        Log("Failed to execute %s", cmd);
        return false;
    }
    return true;
}


#define _STRINGIFY(n) #n

EXPORT_CNC uint32_t Start()
{
    CncRemote::Client client;
    CncString tmp;
    try
    {
        if(!client.Connect(0, "127.0.0.1",DEFAULT_COMMS_PORT)) return false;
        if(client.Ping(100)) return true; //server is already running
    }catch (std::exception& exc)
    {
//        LogFunc(exc.what());
    }
    if(g_serverPath.size() == 0) //no server available
    {
        LogFunc("No server available");
        return false;
    }
    if(!ExecStart(g_serverPath.c_str()))
    {
        LogFunc("Failed to start server");
        return false;
    }

    auto t = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
    while(t < std::chrono::high_resolution_clock::now() && exec)
    {
        while(ExecPoll()){};
        try
        {
            usleep(1000);
            if(client.Connect(0, "127.0.0.1",DEFAULT_COMMS_PORT))
            {
                client.Ping(250);
                return true;
            }
        }catch (std::exception& exc)
        {
    //        LogFunc(exc.what());
        }
    }
    return false;
}

EXPORT_CNC void Stop()
{
    if(exec)
    {
        kill(-execPid, 9);
        pclose2(exec, execPid);
    }
}

EXPORT_CNC const char * GetName()
{
#ifdef _DEBUG
    return ("LinuxCNC (debug)");
#else
    return ("LinuxCNC");
#endif
}

EXPORT_CNC const uint32_t ControlExists(const char * pluginDir, CNCLOGFUNC logFunc)
{
    LogFunc = logFunc;
    CncRemote::Client client;
    try
    {
        if(client.Connect(0, "127.0.0.1",DEFAULT_COMMS_PORT) &&
                client.Ping(100))
        {
            LogFunc("LinuxCNC server is running");
            return true; //server is already running
        }
    }catch (std::exception& exc)
    {
        LogFunc(exc.what());
    }


    string path = pluginDir;
    path += "linuxcncserver/cncremote.sh -c 2>&1";

    if(ExecStart(path.c_str()))
    {
        while(exec)
        {
            ExecPoll();
            usleep(1000);
        }
    }
    if(execRet == 0);
    {
        g_serverPath = string(pluginDir) + "linuxcncserver/cncremote.sh 2>&1";
        return true;
    }
    return false;
}

EXPORT_CNC void Quit()
{
}

EXPORT_CNC void Poll()
{
    ExecPoll();
}
