#include "cncclient.h"
#include <time.h>

#ifdef _USING_WINDOWS
#include "Shlwapi.h"
#endif

namespace CncRemote
{

Client::Client()
{
	m_isConnected = false;
	m_lastHeart = 0;
	m_nextTime = time(NULL) + CONN_TIMEOUT;
}

Client::~Client()
{
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
}

void Client::LoadPlugins(const CncString& path)
{
	if(m_plugins.size() > 0) return; //Only enumerate plugins once

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
        return;
    }
    do
    {
       Plugin plg;
       wstring dllName = path + dir.cFileName;
       plg.handle = LoadLibraryW(dllName.c_str());
       if(!plg.handle) continue;
       plg.Start = (CNCSTARTFUNC) GetProcAddress(plg.handle,"Start");
       plg.GetName = (CNCGETNAMEFUNC) GetProcAddress(plg.handle,"GetName");
       plg.Quit = (CNCQUITFUNC) GetProcAddress(plg.handle,"Quit");
       plg.Poll = (CNCPOLLFUNC) GetProcAddress(plg.handle,"Poll");
       plg.ControlExists = (CNCCONTROLEXISTSFUNC) GetProcAddress(plg.handle,"ControlExists");
       if(!plg.Start ||
          !plg.GetName||
          !plg.Quit ||
          !plg.Poll ||
          !plg.ControlExists)
      {
          FreeLibrary(plg.handle);
      }else
      {
          plg.status = (CONTROLSTATUS)plg.ControlExists();
		  m_plugins.push_back(plg);
      }
    }
    while(FindNextFileW(hFind, &dir) != NULL);
    FindClose(hFind);
#else
/*
	char buf[PATH_MAX];
	memset(buf,0,sizeof(dest)); // readlink does not null terminate!
	if (readlink("/proc/self/exe", buf, PATH_MAX) == -1)
		perror("readlink");
    string path(buf);
    path += "/plugins/";
*/
	auto dir = opendir((path + "\\*.so").c_str());
    if (dir == NULL)
    {
        return;
    }
	auto file = readdir(dir);
	if(file == NULL)
	{
		return;
	}
	do
	{
       Plugin plg;
	   string dllName = path + file->d_name;
		plg.handle = dlopen("./hello.so", RTLD_LAZY);
       if(!plg.handle) continue;
       plg.Start = (STARTFUNC) dlsym(plg.handle,"Start");
       plg.GetName = (GETNAMEFUNC) dlsym(plg.handle,"GetName");
       plg.Quit = (QUITFUNC) dlsym(plg.handle,"Quit");
       plg.Poll = (POLLFUNC) dlsym(plg.handle,"Poll");
       plg.ControlExists = (CONTROLEXISTSFUNC) dlsym(plg.handle,"ControlExists");
       if(!plg.Start ||
          !plg.GetName||
          !plg.Quit ||
          !plg.Poll ||
          !plg.ControlExists ||
		  plg.ControlExists() != ctrlNONE)
      {
          dlclose(plg.handle);
      }else
      {
          m_plugins.push_back(plg);
      }
    } while((file = readdir(dir)) != NULL);
	closedir(dir);
#endif

}


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
    bool ret = false;
	char *buf = NULL;
	while(1)
	{
/*		string identity;
		if(m_isServer)
		{
			int ret = RecvString(identity);
			if(ret < 0)
			{
				break;
			}
		}*/
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
	return ret;
}

bool Client::Connect(const int index, const string address)
{
	m_address.clear();
	if(!m_socket)
	{
		m_socket = zmq_socket(m_context, ZMQ_DEALER);
		if(!m_socket) return false;
		int opt = 0;
		zmq_setsockopt(m_socket, ZMQ_LINGER, &opt, sizeof(opt)); 
	}
	if(zmq_connect(m_socket, address.c_str()) < 0)
	{
		return false;
	}
	m_address = address;
	char* identity = "sheetcam";
    zmq_setsockopt (socket, ZMQ_IDENTITY, identity, strlen (identity));
	Packet pkt;
	pkt.cmd = cmdPING;
	SendPacket(pkt);
	return true;
}

void Client::Disconnect()
{
	if(!m_socket ||
		m_address.empty())
	{
		return;
	}
	zmq_disconnect(m_socket, m_address.c_str());
	m_isConnected = false;
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
