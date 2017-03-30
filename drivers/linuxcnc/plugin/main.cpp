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
    if(!client.Connect(0,"localhost:" _STRINGIFY(DEFAULT_COMMS_PORT))) return false;

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
    return ("LinuxCNC");
}

EXPORT_CNC const uint32_t ControlExists(const char * pluginDir)
{
    string path = pluginDir;
    path += "LinuxCNC";
    DIR* dir = opendir((path).c_str());
    if (dir == NULL)
    {
        return false;
    }
    dirent * file = readdir(dir);
    if(file == NULL)
    {
        return false;
    }
    do
    {
        if(strcmp(file->d_name, ".") == 0 ||
                strcmp(file->d_name, "..") ==0)
        {
            continue;
        }
        string srvName = path + "/" + file->d_name;
        int status = execute(srvName.c_str(), "-c", true);
        if(status == 1)
        {
            g_serverPath = srvName;
            break;
        }
    }
    while((file = readdir(dir)) != NULL);
    closedir(dir);
    return(g_serverPath.size() > 0);
}

EXPORT_CNC void Quit()
{
}

EXPORT_CNC void Poll()
{

}
