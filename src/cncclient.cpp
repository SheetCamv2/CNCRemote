#include "cncclient.h"
#include <time.h>

#ifdef _USING_WINDOWS
#include "Shlwapi.h"
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

#include <sstream>
#include "millisleep.h"

namespace CncRemote
{

Client::Client()
{
    m_isConnected = false;
    m_wasConnected = false;
    m_repTimeout = 0;
#ifdef USE_PLUGINS
    m_plugin = NULL;
#endif
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
        plg.handle = dlopen(dllName.c_str(), RTLD_LAZY);
        if(!plg.handle) continue;
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
    case cmdSTATE:
    {
        StateBuf buf;
        buf.ParseFromString(pkt.data);
        MergeFrom(buf);
    }
    break;
    }
}


bool Client::Poll()
{
    if(m_socket == NULL) return false;
#ifdef USE_PLUGINS
    if(m_plugin) m_plugin->Poll();
#endif
    bool ret = false;
    while(1)
    {

        Packet pkt;
        if(!RecvPacket(pkt))
        {
            break;
        }
        switch(pkt.cmd)
        {
        case cmdNULL:
            break;

        case cmdSENDFILE: //TODO: File handling
            break;

        case cmdREQFILE: //TODO: File handling
            break;

        case cmdPING:
            m_pingResp = true;
            break;

        default:
            HandlePacket(pkt);
        }
        ret = true;
    }
    if(ret)
    {
        m_timeout = time(NULL) + CONN_TIMEOUT;
    }

    m_isConnected = (m_timeout > time(NULL));
    if(m_isConnected)
    {
        SendCommand(cmdSTATE);
    }
    if(m_isConnected != m_wasConnected)
    {
        m_wasConnected = m_isConnected;
        if(!m_isConnected)
        {
printf("Connection timed out\n");
            SendCommand(cmdSTATE); //queue a state message to wake up the server when we connect
        }
else
printf("Regained connection\n");
        OnConnection(m_isConnected);
    }
    return ret;
}

CncString Client::GenerateTcpAddress(const CncString& ipAddress, const bool useLocal, const int port)
{
#ifdef _USING_WINDOWS
    wstringstream stream;
#define _TT(n) L##n
#else
    stringstream stream;
#define _TT(n) n
#endif
    if(useLocal)
    {
        stream <<  _TT("tcp://localhost:") << DEFAULT_COMMS_PORT;
    }
    else
    {
        stream <<  _TT("tcp://") << ipAddress << _TT(":") << port;
    }
    return stream.str();
}

bool Client::Connect(const unsigned int index, const CncString& address)
{
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
#endif
    m_address.clear();
    if(m_socket)
    {
        zmq_close(m_socket);
    }
    if(m_isConnected)
    {
printf("Connection failed\n");
        m_isConnected = false;
        OnConnection(false);
    }
    m_wasConnected = false;
#ifdef USE_PLUGINS
    if(index > 0)
    {
        if(index > m_plugins.size()) return false;
        m_plugin = &m_plugins[index - 1];
    }
#endif
    m_socket = zmq_socket(m_context, ZMQ_DEALER);
    if(!m_socket)
    {
#ifdef USE_PLUGINS
        m_plugin = NULL;
#endif
        return false;
    }
    int opt = 0;
    zmq_setsockopt(m_socket, ZMQ_LINGER, &opt, sizeof(opt));
    if(zmq_connect(m_socket, to_utf8(address).c_str()) < 0)
    {
#ifdef USE_PLUGINS
        m_plugin = NULL;
#endif
        return false;
    }
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Start();
    }
#endif
    m_address = address;
    const char* identity = "sheetcam";
    zmq_setsockopt (m_socket, ZMQ_IDENTITY, identity, strlen (identity));
    Packet pkt;
    pkt.cmd = cmdSTATE;
    SendPacket(pkt);
    return true;
}

void Client::Disconnect()
{
printf("Disconnect\n");
    if(!m_socket ||
            m_address.empty())
    {
        return;
    }
#ifdef USE_PLUGINS
    if(m_plugin)
    {
        m_plugin->Stop();
        m_plugin = NULL;
    }
#endif
    zmq_disconnect(m_socket, to_utf8(m_address).c_str());
    m_isConnected = false;
    OnConnection(false);
}

bool Client::Ping(int waitMs)
{
    m_pingResp = false;
    SendCommand(cmdPING);
    while (!m_pingResp && waitMs >= 0)
    {
        sleep_ms(10);
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
