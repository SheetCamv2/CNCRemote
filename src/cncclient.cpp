#include "cncclient.h"
#include <time.h>

#ifdef _USING_WINDOWS
#include "Shlwapi.h"
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <sstream>
#include "timer.h"

namespace CncRemote
{

Client::Client()
{
#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
    SetSocket(new CActiveSocket());
}

Client::~Client()
{
#ifdef USE_PLUGINS

    for(size_t ct=0; ct < m_plugins.size(); ct++)
    {
        Plugin &plg = m_plugins[ct];
        plg.Quit();
#ifdef _USING_WINDOWS
        FreeLibrary(plg.handle);
#else
        dlclose(plg.handle);
#endif
    }
#endif
}


#ifdef USE_PLUGINS

bool Client::LoadPlugins(const CncString& path)
{
    if(m_plugins.size() > 0) return true; //Only enumerate plugins once

#ifdef _USING_WINDOWS
    /*
    	WCHAR buf[MAX_PATH];
        DWORD length = GetModuleFileNameW( NULL, buf, MAX_PATH );
    //#if (NTDDI_VERSION >= NTDDI_WIN8)
    //    PathCchRemoveFileSpecW(buf, MAX_PATH);
    //#else
        PathRemoveFileSpecW(buf);
    //#endif
        wstring path(buf);
        path += L"\\plugins\\";
    */


    WIN32_FIND_DATAW dir;
    HANDLE hFind = FindFirstFileW((path + L"\\*.dll").c_str(), &dir);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    do
    {
        Plugin plg;
        wstring dllName = path + dir.cFileName;
        plg.handle = LoadLibraryW(dllName.c_str());
        if(!plg.handle) continue;
        plg.Start = (CNCSTARTFUNC) GetProcAddress(plg.handle,"Start");
        plg.Stop = (CNCSTOPFUNC) GetProcAddress(plg.handle,"Stop");
        plg.GetName = (CNCGETNAMEFUNC) GetProcAddress(plg.handle,"GetName");
        plg.Quit = (CNCQUITFUNC) GetProcAddress(plg.handle,"Quit");
        plg.Poll = (CNCPOLLFUNC) GetProcAddress(plg.handle,"Poll");
        plg.ControlExists = (CNCCONTROLEXISTSFUNC) GetProcAddress(plg.handle,"ControlExists");
        if(!plg.Start ||
                !plg.Stop||
                !plg.GetName||
                !plg.Quit ||
                !plg.Poll ||
                !plg.ControlExists ||
                plg.ControlExists() == false)
        {
            FreeLibrary(plg.handle);
        }
        else
        {
            m_plugins.push_back(plg);
        }
    }
    while(FindNextFileW(hFind, &dir) != NULL);
    FindClose(hFind);
#else
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
        Plugin plg;
        if(strcmp(file->d_name, ".") == 0 ||
           strcmp(file->d_name, "..") == 0)
        {
            continue;
        }
        string dllName = path + "/" + file->d_name;
        plg.handle = dlopen(dllName.c_str(), RTLD_NOW);
        if(!plg.handle)
        {
            char * msg = dlerror();
            printf("%s\n", msg);
            continue;
        }
        plg.Start = (CNCSTARTFUNC) dlsym(plg.handle,"Start");
        plg.Stop = (CNCSTOPFUNC) dlsym(plg.handle,"Stop");
        plg.GetName = (CNCGETNAMEFUNC) dlsym(plg.handle,"GetName");
        plg.Quit = (CNCQUITFUNC) dlsym(plg.handle,"Quit");
        plg.Poll = (CNCPOLLFUNC) dlsym(plg.handle,"Poll");
        plg.ControlExists = (CNCCONTROLEXISTSFUNC) dlsym(plg.handle,"ControlExists");
        if(!plg.Start ||
                !plg.Stop||
                !plg.GetName||
                !plg.Quit ||
                !plg.Poll ||
                !plg.ControlExists ||
                plg.ControlExists((path + "/").c_str()) == false)
        {
            dlclose(plg.handle);
        }
        else
        {
            m_plugins.push_back(plg);
        }
    }
    while((file = readdir(dir)) != NULL);
    closedir(dir);
#endif
    return true;
}
#endif

void Client::HandlePacket(const Packet & pkt)
{
    switch(pkt.cmd)
    {
    case cmdNULL:
        break;

    case cmdSTATE:
    {
        StateBuf buf;
        buf.ParseFromString(pkt.data);
int a = pkt.data.size();
        m_state.MergeFrom(buf);
    }
    break;
    }
}


bool Client::Poll()
{
#ifdef USE_PLUGINS
    if(m_plugin) m_plugin->Poll();
#endif

    COMERROR ret = Comms::Poll();
    /*switch(ret)
    {
    case errNODATA:
//        m_isConnected = (m_timeout > time(NULL));
        break;

    case errOK:
//        m_timeout = time(NULL) + CONN_TIMEOUT;
        break;

    default:
    }*/

//    if(m_isConnected)
    {
        SendCommand(cmdSTATE);
    }
    return ret == errOK;
}


bool Client::Connect(const unsigned int index, const CncString& address, const uint32_t port)
{
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
    if(index > 0)
    {
        if(index > m_plugins.size()) return false;
        m_plugin = &m_plugins[index - 1];
        m_plugin->Start();
        Comms::Connect("localhost",port);
    }else
#endif

    Comms::Connect(address,port);
    return true;
}

void Client::Disconnect()
{
printf("Disconnect\n");
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
#endif
    Close();
}

bool Client::Ping(int waitMs)
{
    m_pingResp = false;
    SendCommand(cmdPING);
    while (!m_pingResp && waitMs >= 0)
    {
        SleepMs(10);
        if(!Poll()) return false;
        waitMs -= 10;
    }
    return m_pingResp;
}


void Client::DrivesOn(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdDRIVESON, cmd);
}

void Client::JogVel(const Axes& velocities)
{
    CmdBuf cmd;
    *cmd.mutable_axes() = velocities;
    SendCommand(cmdJOGVEL, cmd);
}

void Client::Mdi(const string line)
{
    CmdBuf cmd;
    cmd.set_string(line);
    SendCommand(cmdMDI, cmd);
}


void Client::SetFRO(const double percent)
{
    CmdBuf cmd;
    cmd.set_rate(percent);
    SendCommand(cmdFRO, cmd);
}

void Client::LoadFile(const string file)
{
    CmdBuf cmd;
    cmd.set_string(file);
    SendCommand(cmdFILE, cmd);
}

void Client::CloseFile()
{
    CmdBuf cmd;
    SendCommand(cmdCLOSEFILE, cmd);
}


void Client::CycleStart()
{
    CmdBuf cmd;
    SendCommand(cmdSTART, cmd);
}

void Client::Stop()
{
    CmdBuf cmd;
    SendCommand(cmdSTOP, cmd);
}

void Client::Pause(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdPAUSE, cmd);
}


void Client::BlockDelete(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdBLOCKDEL, cmd);
}

void Client::SingleStep(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdSINGLESTEP, cmd);
}

void Client::OptionalStop(const bool state)
{
    CmdBuf cmd;
    cmd.set_state(state);
    SendCommand(cmdOPTSTOP, cmd);
}

} //namespace CncRemote
