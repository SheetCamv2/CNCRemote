#include "cnccomms.h"
#include "EdingCNC.h"

#include <tlhelp32.h>

enum
{

    btnEnable = 1000,
    btnHome = 1001,
    btnStart = 1002,
    btnPause = 1003,
    btnStop = 1004,
    btnRewind = 1005,
    btnMist = 1006,
    btnFlood = 1007,
    btnSpindle = 1008,
    btnJogXP = 1009,
    btnJogXM = 1010,
    btnJogYP = 1011,
    btnJogYM = 1012,
    btnJogZP = 1013,
    btnJogZM = 1014,
    btnJogAP = 1015,
    btnJogAM = 1016,
    btnJogBP = 1017,
    btnJogBM = 1018,
    btnJogCP = 1019,
    btnJogCM = 1020,

    btnZeroX = 1021,
    btnZeroY = 1022,
    btnZeroZ = 1023,
    btnZeroA = 1024,
    btnZeroB = 1025,
    btnZeroC = 1026,
    btnJogStep = 1027,
    btnHomeX = 1028,
    btnHomeY = 1029,
    btnHomeZ = 1030,
    btnHomeA = 1031,
    btnHomeB = 1032,
    btnHomeC = 1033,
    btnBlockDelete = 1034,
    btnSingleStep = 1035,
    btnFastJog = 1036,
    btnSyncTools = 1037,
    btnAutoSync = 1038,
    btnMonitorTools = 1039,
    btnSimulate = 1040,

    droX = 2000,
    droY = 2001,
    droZ = 2002,
    droA = 2003,
    droB = 2004,
    droC = 2005,
    droCommandFeed = 2006,
    droFRO = 2007,
    droActualFeed = 2008,
    droSpindle = 2009,
    droMachineX = 2010,
    droMachineY = 2011,
    droMachineZ = 2012,
    droMachineA = 2013,
    droMachineB = 2014,
    droMachineC = 2015,
    droStepSize = 2016,
    droJogPercent = 2017,

    /*use this ID for the panel containing jog buttons. This catches key presses
    NOTE: Any controls on this panel that are not text boxes should have the
    wxWANTS_CHARS style flag set
    */
    funcJOG = 3000,

    /*MDI entry box. Should be a single line text box.
    Must have the wxWANTS_CHARS style flag set
    */
    funcMDIBOX = 3001,

    /*MDI history box. Should be a list box.
    Notes:
    You must have a MDI box for this box to be of any use.
    This box must be on the same panel as the MDI box.
    Must have the wxWANTS_CHARS and wxTE_PROCESS_ENTER style flags set.
    */
    funcMDIHISTORY = 3002,

    /*G and M codes. Should be a multi line text box. Should be read only
    */
    funcGMCODES = 3003,

    /* Status box. Should be a text box. Should be read only*/
    funcSTATUS = 3004,
};


struct ENABLE
{
    int button;
    CURSTATE enable;
};

static const ENABLE g_enables[]=
{
    {btnEnable, stateANY},
    {btnHome, stateIDLE},
    {btnStart, statePAUSED},
    {btnPause, stateRUNNING},
    {btnStop, stateANY},
    {btnRewind, stateIDLE},
    {btnMist, stateANY},
    {btnFlood, stateANY},
    {btnSpindle, stateTOOLCHANGE},
    {btnJogXP, statePAUSED},
    {btnJogXM, statePAUSED},
    {btnJogYP, statePAUSED},
    {btnJogYM, statePAUSED},
    {btnJogZP, statePAUSED},
    {btnJogZM, statePAUSED},
    {btnJogAP, statePAUSED},
    {btnJogAM, statePAUSED},
    {btnJogBP, statePAUSED},
    {btnJogBM, statePAUSED},
    {btnJogCP, statePAUSED},
    {btnJogCM, statePAUSED},
    {btnZeroX, stateTOOLCHANGE},
    {btnZeroY, stateTOOLCHANGE},
    {btnZeroZ, stateTOOLCHANGE},
    {btnZeroA, stateTOOLCHANGE},
    {btnZeroB, stateTOOLCHANGE},
    {btnZeroC, stateTOOLCHANGE},
    {btnJogStep, statePAUSED},
    {btnHomeX, stateIDLE},
    {btnHomeY, stateIDLE},
    {btnHomeZ, stateIDLE},
    {btnHomeA, stateIDLE},
    {btnHomeB, stateIDLE},
    {btnHomeC, stateIDLE},
    {btnBlockDelete, statePAUSED},
    {btnSingleStep, statePAUSED},
    {btnFastJog, statePAUSED},
    {btnSyncTools, stateIDLE},
    {btnAutoSync, stateIDLE},
    {btnMonitorTools, stateIDLE},
    {btnSimulate, stateIDLE},

    {droX, stateTOOLCHANGE},
    {droY, stateTOOLCHANGE},
    {droZ, stateTOOLCHANGE},
    {droA, stateTOOLCHANGE},
    {droB, stateTOOLCHANGE},
    {droC, stateTOOLCHANGE},
    {droCommandFeed, stateNEVER},
    {droFRO, stateANY},
    {droActualFeed, stateNEVER},
    {droSpindle, stateRUNNING},
    {droMachineX, stateNEVER},
    {droMachineY, stateNEVER},
    {droMachineZ, stateNEVER},
    {droMachineA, stateNEVER},
    {droMachineB, stateNEVER},
    {droMachineC, stateNEVER},
    {droStepSize, stateANY},
    {droJogPercent, stateANY},

    {funcJOG, statePAUSED},
    {funcMDIBOX, stateTOOLCHANGE},
    {funcMDIHISTORY, stateTOOLCHANGE},
    {funcGMCODES, stateNEVER},
    {funcSTATUS, stateNEVER},
    {-1,stateNEVER}
};

EdingCnc::EdingCnc()
{
    m_autoLoad = false;
    m_controlCount = 0;
    m_displayOn = false;
    m_step = false;
    m_jogVel = 0.25;
    m_stepSize = 1;
    m_oldLine = -1;
    memset(&m_jogging,0,sizeof (CNC_CART_DOUBLE));
    m_fastJog = false;
    m_toolCount = 0;
    m_autoSync = false;
    m_autoMonitor = false;
    m_connected = false;
    m_connectCount = 0;
    m_curState = stateNEVER;
    m_zMax = -1e17;
    m_cncDll = NULL;
}

EdingCnc::~EdingCnc()
{
    if(m_cncDll) FreeLibrary(m_cncDll);
}

#define GETSYMBOL(type, f)     f = (type)GetProcAddress(m_cncDll,L(#f)); if(!f) ok = false;
#define GETSYMBOL2(type, f, a) f = (type)GetProcAddress(m_cncDll,L(#a)); if(!f) ok = false;


void EdingCnc::GetPausePos(CNC_CART_DOUBLE *pos, int *spindleOutput, int *FloodOutput, int *mistOutput,
                           int *lineNumber, int *valid, int* doArray, int *arrayX, int *arrayY)
{
    CNC_PAUSE_STS* sts = CncGetPauseStatus();
    memcpy(pos, &sts->pausePosition, sizeof(CNC_CART_DOUBLE));
    *spindleOutput = sts->pauseSpindleIOValue;
    *FloodOutput = sts->pauseFloodIOValue;
    *mistOutput = sts->pauseMistIOValue;
    *lineNumber = sts->pausePositionLine;
    *valid = sts->pausePositionValid;
    *doArray = sts->pauseDoArray;
    *arrayX = sts->pauseArrayIndexX;
    *arrayY = sts->pauseArrayIndexY;
}


bool EdingCnc::LoadDll()
{
    if(m_cncDll)
    {
        return true;
    }
    wstring path;
    HKEY key;
    wstring regPath = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    LONG ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
    if(ret != ERROR_SUCCESS)
    {
        regPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
    }
    if(ret != ERROR_SUCCESS) return false; //something has gone horribly wrong
    DWORD    subKeys=0;               // number of subkeys


    // Get the class name and the value count.
    ret = RegQueryInfoKey(
              key,                     // key handle
              NULL,                // buffer for class name
              NULL,           // size of class string
              NULL,                    // reserved
              &subKeys,               // number of subkeys
              NULL,            // longest subkey size
              NULL,            // longest class string
              NULL,                // number of values for this key
              NULL,            // longest value name
              NULL,         // longest value data
              NULL,   // security descriptor
              NULL);       // last write time


    if(!subKeys)
    {
        RegCloseKey(key);
        return false;
    }
    TCHAR    keyName[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    nameSize;                   // size of name string

    int dllVersion = 0;
    for (int i=0; i<subKeys; i++)
    {
        ret = RegEnumKeyEx(key, i,keyName, &nameSize,  NULL, NULL, NULL, NULL);
        if (ret == ERROR_SUCCESS)
        {
            if(wcsncmp(keyName,L"CNC4.0",6) != 0) continue;
            int ver = keyName[6] - L'0';
            if(ver < dllVersion) continue;
            DWORD    values=0;
            HKEY key2;
            ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, (regPath + L"\\" + keyName).c_str(), 0, KEY_READ, &hKey);
            if(ret != ERROR_SUCCESS) continue;
            TCHAR keyData[MAX_KEY_LENGTH];
            DWORD dataSize = MAX_KEY_LENGTH;
            ret = RegQueryValueExW(hKey, L"Publisher", 0, NULL, keyData, &dataSize);
            if(ret != ERROR_SUCCESS ||
                    wcscmp(keyData,L"EDING CNC B.V.") != 0)
            {
                RegCloseKey(key2);
                continue;
            }
            ret = RegQueryValueExW(hKey, L"InstallLocation", 0, NULL, keyData, &dataSize);
            if(ret == ERROR_SUCCESS)
            {
                dllVersion = ver;
                path = keyData;
            }
            RegCloseKey(key2);
        }
    }
    RegCloseKey(key);
    if(path.empty())
    {
        return false;
    }
    path += L"\\cncapi.dll"
    m_cncDll = LoadLibraryW(path.c_str())
    if(!m_cncDll)
    {
        return false;
    }

    bool ok = true;
    GETSYMBOL2(CNCGETCURRENTT, CncGetCurrentT, CncGetCurrentToolNumber);
    GETSYMBOL2(CNCGETBLOCKDELETE, CncGetBlockDelete, CncGetBlocDelete);
    GETSYMBOL2(CNCGETWORKPOS, CncGetWorkPos, CncGetWorkPosition);
    GETSYMBOL2(CNCGETMACHINEPOS, CncGetMachinePos, CncGetMachinePosition);

    GETSYMBOL(CNCGETPAUSESTATUS, CncGetPauseStatus);
    GETSYMBOL(CNCGETSPINDLESTATUS, CncGetSpindleStatus);

    GETSYMBOL(CNCSETTRACKINGVELOCITY, CncSetTrackingVelocity);
    GETSYMBOL(CNCSTARTVELOCITYTRACKING, CncStartVelocityTracking);
    GETSYMBOL(CNCSTOPTRACKING, CncStopTracking);
    GETSYMBOL(CNCGETJOINTCONFIG, CncGetJointConfig);

    GETSYMBOL(CNCGETTOOLDATA, CncGetToolData);
    GETSYMBOL(CNCISSERVERCONNECTED, CncIsServerConnected);
    GETSYMBOL(CNCCONNECTSERVER, CncConnectServer);
    GETSYMBOL(CNCDISCONNECTSERVER, CncDisConnectServer);
    GETSYMBOL(CNCLOADJOB, CncLoadJob);
    GETSYMBOL(CNCLOADTOOLTABLE, CncLoadToolTable);
    GETSYMBOL(CNCUPDATETOOLDATA, CncUpdateToolData);
    GETSYMBOL(CNCGETSTATE, CncGetState);
    GETSYMBOL(CNCGETSTATETEXT, CncGetStateText);
    GETSYMBOL(CNCGETCURINTERPRETERLINE, CncGetCurInterpreterLine);
    GETSYMBOL(CNCGETCUREXECLINE, CncGetCurExecLine);
    GETSYMBOL(CNCGETJOINTSTATUS, CncGetJointStatus);
    GETSYMBOL(CNCGETSIMULATIONMODE, CncGetSimulationMode);
    GETSYMBOL(CNCGETALLAXESHOMED, CncGetAllAxesHomed);
    GETSYMBOL(CNCGETSINGLESTEPMODE, CncGetSingleStepMode);
    GETSYMBOL(CNCGETACTUALFEEDOVERRIDE, CncGetActualFeedOverride);
    GETSYMBOL(CNCGETACTUALFEED, CncGetActualFeed);
    GETSYMBOL(CNCGETIOSTATUS, CncGetIOStatus);
    GETSYMBOL(CNCGETCURRENTMCODESTEXT, CncGetCurrentMcodesText);
    GETSYMBOL(CNCGETCURRENTGCODESTEXT, CncGetCurrentGcodesText);
    GETSYMBOL(CNCRUNSINGLELINE, CncRunSingleLine);
    GETSYMBOL(CNCSTARTJOG, CncStartJog);
    GETSYMBOL(CNCSETOUTPUT, CncSetOutput);
    GETSYMBOL(CNCSETFEEDOVERRIDE, CncSetFeedOverride);
    GETSYMBOL(CNCREWINDJOB, CncRewindJob);
    GETSYMBOL(CNCRESET, CncReset);
    GETSYMBOL(CNCRESET2, CncReset2);
    GETSYMBOL(CNCGETTRACKINGSTATUS, CncGetTrackingStatus);
    GETSYMBOL(CNCPAUSEJOB, CncPauseJob);
    GETSYMBOL(CNCRUNORRESUMEJOB, CncRunOrResumeJob);
    GETSYMBOL(CNCSETSIMULATIONMODE, CncSetSimulationMode);
    GETSYMBOL(CNCSINGLESTEPMODE, CncSingleStepMode);
    GETSYMBOL(CNCENABLEBLOCKDELETE, CncEnableBlockDelete);
    GETSYMBOL(CNCSTOPJOG, CncStopJog);
    GETSYMBOL(CNCGETINPUT, CncGetInput);
    GETSYMBOL(CNCABORTJOB, CncAbortJob);
    GETSYMBOL(CNCGETEMSTOPACTIVE, CncGetEMStopActive);
    GETSYMBOL(CNCMOVETO, CncMoveTo);
    GETSYMBOL(CNCGETACTUALTOOLZOFFSET, CncGetActualToolZOffset);
    GETSYMBOL(CNCGETACTUALORIGINOFFSET, CncGetActualOriginOffset);

    if(!ok)
    {
        FreeLibrary(m_cncDll);
    }
    return ok;
}

void EdingCnc::OnConnect(void)
{
    m_displayOn = true;
    scPlugin->GetSetting(_T("jogVel"), &m_jogVel);
    scPlugin->GetSetting(_T("stepSize"), &m_stepSize);
    scPlugin->GetSetting(_T("autoSync"), &m_autoSync);
    scPlugin->GetSetting(_T("autoMonitor"), &m_autoMonitor);
}


void EdingCnc::PollConnected()
{
    if(!GuiRunning())
    {
        m_connectCount = 0;
        if(m_connected)
        {
            m_connected = false;
            scPlugin->NotifyLua(dllENABLE, enableNOTRUNNING);
            CncDisConnectServer();
            m_cncDll.Unload();
        }
        return;
    }
    if(m_connected)
    {
        return;
    }
    if(m_connectCount < 4)
    {
        m_connectCount++;
        return;
    }
    if(!LoadDll())
    {
        return;
    }
    CNC_RC res = CncConnectServer("");
    if(res != CNC_RC_ALREADY_RUNS)
    {
        /*		if(CncIsServerConnected())
        		{
        			CncDisConnectServer();
        		}*/
        return;
    }
    if(!CncIsServerConnected())
    {
        return;
    }
    m_connected = true;
    scPlugin->NotifyLua(dllENABLE,enableRUNNING);
    for(int ct=0; ct< CNC_MAX_TOOLS; ct++)
    {
        m_tools[ct] = CncGetToolData(ct);
    }
}

bool EdingCnc::GuiRunning()
{
    PROCESSENTRY32 procEntry;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        return false ;
    }
    procEntry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnap, &procEntry))
    {
        CloseHandle(hSnap);
        return false ;
    }
    int found = 0;

    do
    {
        if (_tcsicmp(procEntry.szExeFile, TEXT("usbcnc.exe")) == 0 ||
                _tcsicmp(procEntry.szExeFile, TEXT("cnc.exe")) == 0)
        {
            found |= 1;
        }
        if (_tcsicmp(procEntry.szExeFile, TEXT("CncServer.exe")) == 0)
        {
            found |= 2;
        }
        if(found == 3)
        {
            CloseHandle(hSnap);
            return(true);
        }

    }
    while (Process32Next(hSnap, &procEntry));

    CloseHandle(hSnap);
    return(false);
}


int EdingCnc::OnNotify(const int index,const double val)
{
    if(index >= dllCONTROL_CHG)
    {
        ControlChanged((index - dllCONTROL_CHG) % 10000, val);
        return(0);
    }

    switch(index)
    {
    case dllTIMER:
        DoTimer();
        break;

    case dllENABLE:
        m_autoLoad = (int)val;
        break;

    case dllCONTROL_NUM:
        if(m_controlCount >= MAX_CONTROLS)
        {
            return(-1);
        }
        {
            CONTROL& ctrl = m_controls[m_controlCount++];
            ctrl.enabled = true;
            ctrl.value = 1e17;
            ctrl.number = val;
            ctrl.enState = stateANY;
            const ENABLE * ptr = g_enables;
            while(ptr->button > 0)
            {
                if(ptr->button == ((int)val) % 10000)
                {
                    ctrl.enState = ptr->enable;
                    break;
                }
                ptr++;
            }
        }
        break;

    case dllMDI:
    {
        scChar buf [1024];
        if(scPlugin->GetSetting(_T("MDIText"), buf, 1024) && buf[0] !=0)
        {
            wstring txt(buf);
            MDI(txt.ToAscii());
        }
    }
    break;

    }
    return(0);
}

void EdingCnc::SendString(const int index, const wstring value)
{
    CONTROL * ptr = m_controls;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        if(index == ptr->number)
        {
            scPlugin->SetSetting(_T("stringData"), value.c_str());
            scPlugin->NotifyLua(dllSTRING, ct);
        }
        ptr++;
    }
}

void EdingCnc::StatusText(const wstring& msg)
{
    SendString(funcSTATUS, msg);
}


void EdingCnc::CheckDro(const int index, const double value)
{
    CONTROL * ptr = m_controls;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        if(index == ptr->number)
        {
            if(value != ptr->value)
            {
                scPlugin->NotifyLua(ct + dllCONTROL_CHG, value);
                ptr->value = value;
                switch(index)
                {
                case btnHome:
                    if(!value)
                    {
                        StatusText(_T("Not homed."));
                    }
                    break;
                }
            }
        }
        ptr++;
    }
}

bool EdingCnc::OnPostBegin(const scChar * fileName, const bool local)
{
    if(local || !m_autoLoad || !CncIsServerConnected())
    {
        return(false);
    }

    CncLoadJob(_T(""));
    return(false);
}

void EdingCnc::PopulateTool(CNC_TOOL_DATA& data, RoundTool * tool)
{
    data.id = tool->number;
    wstring name = tool->name;
    name.Truncate(79);
    strcpy(data.description,name.ToAscii());
    data.diameter = tool->diameter;
    data.orientation = 9;
    data.xOffset = 0;
    RotaryTool * rot = CASTROTARYTOOL(tool);
    if(rot)
    {
        data.zOffset = rot->lengthOffset;
    }
    else
    {
        data.zOffset = 0;
    }
}

void EdingCnc::SyncTools()
{
    Tools& tools = Parts::Get().tools;
    const wxCharBuffer unused = wstring(_T("Unused")).ToAscii();
    for(int ct=1; ct< CNC_MAX_TOOLS; ct++)
    {
        bool found = false;
        for(int t=0; t< tools.GetCount(); t++)
        {
            RoundTool * rTool = CASTROUNDTOOL(tools[t]);
            if(rTool && rTool->number == ct)
            {
                found = true;
                break;
            }
        }
        if(found)
        {
            continue;
        }
        if(strcmp(m_tools[ct].description, unused) != 0)
        {
            strcpy(m_tools[ct].description,unused);
            CncUpdateToolData(&m_tools[ct], ct);
        }
    }
    for(int ct=0; ct < tools.GetCount(); ct++)
    {
        ToolBase* tool = tools[ct];
        RoundTool * rTool = CASTROUNDTOOL(tool);
        int num = rTool->number;
        if(rTool && num < CNC_MAX_TOOLS && num > 0)
        {
            CNC_TOOL_DATA data;
            PopulateTool(data, rTool);
            if(memcmp(&data, &m_tools[num], sizeof(CNC_TOOL_DATA)) != 0)
            {
                CncUpdateToolData(&data, num);
                m_tools[num] = data;
            }
        }
    }
    CncLoadToolTable();
}


void EdingCnc::OnPostEnd(const scChar * fileName, const bool local)
{

    if(local || !m_autoLoad || fileName[0] ==0 || !CncIsServerConnected())
    {
        return;
    }

    if(CncGetState() != CNC_IE_READY_STATE)
    {
        wxMessageBox(_T("Eding CNC is busy. Code not loaded"));
        return;
    }


    CncLoadJob(fileName);
    if(m_autoSync)
    {
        SyncTools();
    }
}


void EdingCnc::ScanTools()
{
    Tools& tools = Parts::Get().tools;
    m_toolCount++;
    if(m_toolCount >= tools.GetCount())
    {
        m_toolCount = 0;
    }
    RoundTool * tool = CASTROUNDTOOL(tools[m_toolCount]);
    if(tool)
    {
        int idx = tool->number;
        if(idx > 0 && idx < CNC_MAX_TOOLS)
        {
            CNC_TOOL_DATA src = CncGetToolData(idx);
            CNC_TOOL_DATA& dest = m_tools[idx];
            int dirty = 0;
            if(memcmp(&src, &dest, sizeof(CNC_TOOL_DATA)) != 0)
            {
                if(dest.zOffset != src.zOffset)
                {
                    RotaryTool * rTool = CASTROTARYTOOL(tool);
                    if(rTool)
                    {
                        rTool->lengthOffset = src.zOffset;
                    }
                }
                if(dest.diameter != src.diameter)
                {
                    tool->diameter = src.diameter;
                    dirty = tool->diameter.GetDirtyFlags();
                }
                memcpy(&dest, &src, sizeof(CNC_TOOL_DATA));
            }
            if(dirty)
            {
                Parts::Get().SetDirty(dirty | DIRTY_FILE | DIRTY_TOOLS);
            }
        }
    }
}

//enum CURSTATE {stateNEVER, stateIDLE, statePAUSED, stateRUNNING, stateJOG, stateANY};
void EdingCnc::EnableControls()
{
    static CNC_IE_STATE prevState = CNC_IE_LAST_STATE;
    CURSTATE cur = stateJOG;
    CNC_IE_STATE state = CncGetState();
    if(state == prevState)
    {
        return;
    }
    prevState = state;
    switch(state)
    {
    case CNC_IE_EXEC_ERROR_STATE:
    case CNC_IE_INT_ERROR_STATE:
    case CNC_IE_ABORTED_STATE:
    case CNC_IE_RUNNING_JOB_STATE:
        StatusText(wstring::FromAscii(CncGetStateText(state)));
        break;

    }
    if(state < CNC_IE_RUNNING_JOB_STATE)
    {
        cur = stateIDLE;
    }
    else if (state < CNC_IE_PAUSED_LINE_STATE)
    {
        cur = stateRUNNING;
    }

    else if (state < CNC_IE_RUNNING_HANDWHEEL_STATE)
    {
        CNC_PAUSE_STS* sts = CncGetPauseStatus();
        if(m_curState != statePAUSED &&
                sts->pauseManualActionRequired)
        {
            int tool = CncGetCurrentT();
            Tools& tools = Parts::Get().tools;
            wstring toolTxt = wstring::FromAscii(m_tools[tool].description);
            ToolBase * tl = tools.GetTool(tool);
            if(tl)
            {
                toolTxt = tl->GetName(false);
            }
            StatusText(wstring::Format(_T("Load tool %i: %s"),tool, toolTxt.c_str()));
        }
        cur = statePAUSED;
    }
    /*	}else
    	{
    		if(m_curState != stateTOOLCHANGE)
    		{
    			StatusText(wstring::Format(_T("Load tool %i"),CncGetCurrentT()));
    		}
    		cur = stateTOOLCHANGE;
    		prevState = CNC_IE_LAST_STATE;
    	}*/

    if( cur == m_curState)
    {
        return;
    }
    m_curState = cur;
    for(int ct=0; ct< m_controlCount; ct++)
    {
        CONTROL& ctrl = m_controls[ct];
        bool en = cur <= ctrl.enState;
        if(en != ctrl.enabled)
        {
            ctrl.enabled = en;
            if(en)
            {
                scPlugin->NotifyLua(dllENABLECTRL, ct);
            }
            else
            {
                scPlugin->NotifyLua(dllDISABLECTRL, ct);
            }
        }
    }
}

void EdingCnc::DoTimer(void)
{
    PollConnected();
    if(!m_connected)
    {
        return;
    }

    CheckDro(droJogPercent, m_jogVel * 100); //these two only really need to fire once...
    CheckDro(droStepSize, m_stepSize);
    CheckDro(btnAutoSync, m_autoSync);
    CheckDro(btnMonitorTools, m_autoMonitor);

    EnableControls();

    char buf[80];
    buf[0] = 0;
    CncGetCurrentGcodesText(buf);
    wstring strg = wstring::FromAscii(buf);
    CncGetCurrentMcodesText(buf);
    strg += _T("\n") + wstring::FromAscii(buf);
    if(strg != m_activeCodes)
    {
        m_activeCodes = strg;
        SendString(funcGMCODES, m_activeCodes);
    }

    for(int ct=0; ct< CNC_IOID_LAST; ct++)
    {
        CNC_IO_PORT_STS* s = CncGetIOStatus((CNC_IO_ID)ct);
        if(!s)
        {
            continue;
        }
        CheckDro(ct, s->lvalue);
        if(ct == CNC_IOID_TOOL_OUT)
        {
            CheckDro(btnSpindle, s->lvalue);
        }
    }

    CNC_CART_DOUBLE pos = CncGetMachinePos();
    double *posPtr = (double *) &pos;
    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CheckDro(ct + droMachineX, *posPtr);
        if(ct == 2) //Z axis
        {
            if(*posPtr > m_zMax)
            {
                m_zMax = *posPtr;
            }
        }
        posPtr++;
    }


    pos = CncGetWorkPos();
    posPtr = (double *) &pos;

    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CheckDro(ct + droX, *posPtr);
        posPtr++;
    }

    CheckDro(droActualFeed, CncGetActualFeed());

    CNC_SPINDLE_STS* sts = CncGetSpindleStatus();

    CheckDro(droSpindle, sts->actualSpindleSpeedSigned);

    CheckDro(droFRO, CncGetActualFeedOverride() * 100);

    CheckDro(btnBlockDelete, CncGetBlockDelete());
    CheckDro(btnSingleStep, CncGetSingleStepMode());
    CheckDro(btnHome, CncGetAllAxesHomed());
    CheckDro(btnSimulate, CncGetSimulationMode());

    posPtr = (double *) &pos;
    bool changed = false;
    for(int ct=0; ct < DRAW_AXES; ct++)
    {
        if(*posPtr != m_drawPos[ct])
        {
            changed = true;
            m_drawPos[ct] = *posPtr;
        }
        posPtr++;
    }

    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        CNC_JOINT_STS * joint = CncGetJointStatus(ct);
        CheckDro(ct + btnHomeX, joint->isHomed);
        posPtr++;
    }

    if(changed)
    {
        scPlugin->GlRedraw();
    }
    int line = CncGetCurExecLine() - 1;
    if(line < 0)
    {
        line = CncGetCurInterpreterLine() - 1;
    }
    if(line != m_oldLine)
    {
        m_oldLine = line;
        scPlugin->NotifyLua(dllLINENUMBER, line);
    }
    if(m_autoMonitor)
    {
        ScanTools();
    }
}

void EdingCnc::OnDrawDisplay(void)
{
    if(!m_displayOn || m_drawPos[0] == 1e17)
    {
        return;
    }
    glPushMatrix();
    glTranslatef(m_drawPos[0], m_drawPos[1] ,m_drawPos[2]);
    glColor3f(0.2,0.3,0.3);
    glBegin(GL_LINES);
    glVertex3f(-100000,0,0);
    glVertex3f(100000,0,0);
    glVertex3f(0,-100000,0);
    glVertex3f(0,100000,0);
    glEnd();
    glPopMatrix();
}

void EdingCnc::MDI(const char * string)
{
    CncRunSingleLine((char *)string);
}

void EdingCnc::MDI(const char * string, const double value)
{
    char buf[1024];
    sprintf(buf,string,value);
    CncRunSingleLine(buf);
}


#define ASARRAYD(n) ((double *)&n)
#define ASARRAYB(n) ((int *)&n)

void EdingCnc::JogAxis(int button, bool run)
{
    int axis = button - btnJogXP;
    if(axis < 0)
    {
        return;
    }
    bool neg = axis & 1;
    axis /=2;
    if(axis >= CNC_MAX_AXES)
    {
        return;
    }
    double dir = neg ? -1 : 1;
    double vel = m_jogVel;
    if(m_fastJog)
    {
        vel = 1;
    }

    bool wasMoving = false;
    for(int ct=0; ct< CNC_MAX_AXES; ct++)
    {
        if(ASARRAYD(m_jogging)[ct] != 0)
        {
            wasMoving = true;
        }
    }

    if(run)
    {
        double dir = neg ? -1 : 1;
        if(CncStartVelocityTracking && ! m_step)
        {
            CNC_JOINT_CFG* cfg = CncGetJointConfig(axis);
            ASARRAYD(m_jogging)[axis] = (vel * dir) * cfg->maxVelocity;
            if(!wasMoving)
            {
                CncStartVelocityTracking();
            }
            CNC_CART_BOOL move;
            for(int ct=0; ct< CNC_MAX_AXES; ct++)
            {
                ASARRAYB(move)[ct] = true;
            }
            CncSetTrackingVelocity(m_jogging, move);
        }
        else
        {
            ASARRAYD(m_jogging)[axis] = m_stepSize * dir;
            CncStartJog(ASARRAYD(m_jogging), vel, !m_step);
        }
    }
    else
    {
        ASARRAYD(m_jogging)[axis] = 0;
        if(CncStartVelocityTracking && ! m_step)
        {
            CNC_CART_BOOL move;
            bool moving = false;
            for(int ct=0; ct< CNC_MAX_AXES; ct++)
            {
                moving |= ASARRAYD(m_jogging)[ct] != 0;
                ASARRAYB(move)[ct] = true;
            }
            if(moving)
            {
                CncSetTrackingVelocity(m_jogging, move);
                wxLogMessage(_T("x %f y %f"), m_jogging.x, m_jogging.y);
            }
            else
            {
                CncStopTracking();
            }
        }
        else
        {
            CncStopJog(axis);
        }
        /*		bool moving = false;
        		for(int ct=0; ct< CNC_MAX_AXES; ct++)
        		{
        			if(m_jogging.array[ct] == 0)
        			{
        				CncStopJog(ct);
        			}else
        			{
        				moving = true;
        			}
        		}
        		if(moving && !m_step)
        		{
        			CncStartJog(m_jogging, vel, !m_step);
        		}*/
    }
}


void EdingCnc::Toggle(CNC_IO_ID pin)
{
    CNC_IO_PORT_STS* s = CncGetIOStatus(pin);
    CncSetOutput(pin, !s->lvalue);
}


void EdingCnc::DrivesOnOff()
{
    StatusText(_T(""));
    CNC_IE_STATE state = CncGetState();
    if (CncGetEMStopActive())
    {
        return;
    }
    switch (state)
    {
    case CNC_IE_POWERUP_STATE:
    case CNC_IE_IDLE_STATE:
//		 return;

    case CNC_IE_EXEC_ERROR_STATE:
    case CNC_IE_INT_ERROR_STATE:
    case CNC_IE_RENDERING_GRAPH_STATE:
    case CNC_IE_PAUSED_LINE_STATE:
    case CNC_IE_PAUSED_JOB_STATE:
    case CNC_IE_ABORTED_STATE:
        //These are all exceptional states, reset and done
        CncReset2(1);
        break;

    case CNC_IE_RUNNING_HANDWHEEL_STATE:
    case CNC_IE_RUNNING_LINE_HANDWHEEL_STATE:
    case CNC_IE_PAUSED_SUB_SEARCH_STATE:
    case CNC_IE_PAUSED_LINE_SEARCH_STATE:
    case CNC_IE_SEARCHED_DONE_STATE:
    case CNC_IE_READY_STATE:
        //These are all exceptional states, reset and done
//		CncReset2(0);
        CncAbortJob();
        CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 0);
        return;
        break;

    case CNC_IE_RUNNING_JOB_STATE:
    case CNC_IE_RUNNING_LINE_STATE:
    case CNC_IE_RUNNING_SUB_STATE:
    case CNC_IE_RUNNING_LINE_SEARCH_STATE:
    case CNC_IE_RUNNING_SUB_SEARCH_STATE:
        //Running state, Pause and reset, return
        CncPauseJob();
        CncReset2(1);
        break;

    case CNC_IE_RUNNING_AXISJOG_STATE:
    case CNC_IE_RUNNING_IPJOG_STATE:
        //These states cannot be reset
        CncStopJog(-1);
        CncReset2(1);
        break;

    default:
        break;
    }

    CncSetOutput(CNC_IOID_DRIVE_ENABLE_OUT, 1);

    //If we reach here, the state was CNC_IE_READY_STATE
    //Do the reset

    //Check handwheel mode from server status and force back to non tracking mode if needed.
    CNC_TRACKING_STATUS trackSts = *CncGetTrackingStatus();
    bool serverHandwheelMode = false;

    //Update tracking if hand wheel
    if (trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_POS || trackSts.curTrackingMode == CNC_TRACKMODE_HANDWHEEL_VEL)
    {
        serverHandwheelMode = true;
        CncStopTracking();
    }


//    ChangeJogMode(CNC_JOGMODE_CONT, m_curHandwheelAxis, true);

    if (CncGetInput(CNC_IOID_DRIVE_ENABLE_OUT) == 1)
    {
        if (CncGetAllAxesHomed())
        {
            StatusText(_("Drives enabled."));
        }
        else
        {
            wstring msg = _("Drives enabled.");
            msg += _T(" ");
            msg += _("Not homed.");
            StatusText(msg);
        }
    }
    else
    {
        StatusText(_("Drives NOT enabled."));
    }
}

bool EdingCnc::CheckPause()
{

    CNC_PAUSE_STS* sts = CncGetPauseStatus();
    if(sts->pauseManualActionRequired)
    {
        return(true);
    }
    CNC_CART_DOUBLE pausePos;
    int spindle;
    int flood;
    int mist;
    int line;
    int valid;
    int doArray[CNC_MAX_AXES];
    int arrayX[CNC_MAX_AXES];
    int arrayY[CNC_MAX_AXES];
    GetPausePos(&pausePos, &spindle, &flood, &mist, &line, &valid, doArray, arrayX, arrayY);
    if(!valid)
    {
        return(false);
    }

    CNC_CART_DOUBLE offset = CncGetActualOriginOffset();
    offset.z += CncGetActualToolZOffset();

    static CNC_CART_DOUBLE machinePos;
    machinePos = CncGetMachinePos();
    if(memcmp(&pausePos, &machinePos, sizeof(CNC_CART_DOUBLE)) == 0)
    {
        return(true);
    }

    if(m_zMax < machinePos.z)
    {
        m_zMax = machinePos.z;
    }
    if(m_zMax < pausePos.z)
    {
        m_zMax = pausePos.z;
    }

    wstring msg;
    if(machinePos.x != pausePos.x ||
            machinePos.y != pausePos.y)
    {
        msg += wstring::Format(_("Z to %.3f\n"), m_zMax - offset.z);
        msg += wstring::Format(_("X,Y to %.3f, %.3f\n"), pausePos.x - offset.x, pausePos.y - offset.y);
    }
    msg += wstring::Format(_("Z to %.3f\n"), pausePos.z - offset.z);
    msg = _("The machine move sequence will be:\n") + msg;

    int ans = wxMessageBox(msg, _("Safety move"), wxOK | wxCANCEL | wxICON_QUESTION);
    if(ans != wxOK)
    {
        return(false);
    }
    CNC_CART_BOOL moveArray;

    for(int ct=0; ct < CNC_MAX_AXES; ct++)
    {
        ASARRAYB(moveArray)[ct] = 1;
    }
    //={1,1,1,1,1,1};

    if(machinePos.x != pausePos.x ||
            machinePos.y != pausePos.y)
    {
        machinePos.z = m_zMax;
        CncMoveTo(machinePos, moveArray, 0.2);
    }
    if(!WaitMove())
    {
        return(false);
    }
    CncSetOutput(CNC_IOID_COOLANT2_OUT, mist);
    CncSetOutput(CNC_IOID_COOLANT1_OUT, flood);
    CncSetOutput(CNC_IOID_TOOL_OUT, spindle);
    machinePos.x = pausePos.x;
    machinePos.y = pausePos.y;
    machinePos.a = pausePos.a;
    machinePos.b = pausePos.b;
    machinePos.c = pausePos.c;
    CncMoveTo(machinePos, moveArray, 0.2);
    if(!WaitMove())
    {
        return(false);
    }
    machinePos.z = pausePos.z;
    CncMoveTo(machinePos, moveArray, 0.2);
    if(!WaitMove())
    {
        return(false);
    }
    return(true);
}

bool EdingCnc::WaitMove()
{
    CNC_IE_STATE state;
    do
    {
        wxMilliSleep(50);
        wxTheApp->Yield(true);
        state = CncGetState();
    }
    while(state == CNC_IE_RUNNING_AXISJOG_STATE);
    return(state == CNC_IE_PAUSED_JOB_STATE);
}
