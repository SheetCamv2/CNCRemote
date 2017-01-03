#ifndef EDINGCNC_PLUGIN_H
#define EDINGCNC_PLUGIN_H


#define MAX_CONTROLS 100

enum CURSTATE {stateNEVER, stateIDLE, stateTOOLCHANGE, stateJOG, statePAUSED, stateRUNNING, stateANY};

struct CONTROL {
	int number;
	double value;
	bool enabled;
	CURSTATE enState;
};

#define DRAW_AXES 3

class EdingCnc
{
public:
    EdingCnc();
    ~EdingCnc();
	CNCGETPAUSESTATUS CncGetPauseStatus;
	CNCGETSPINDLESTATUS CncGetSpindleStatus;
	bool LoadDll();
    void Poll();


private:
	void GetPausePos(CNC_CART_DOUBLE *pos, int *spindleOutput, int *FloodOutput, int *mistOutput,
		int *lineNumber, int *valid, int* doArray, int *arrayX, int *arrayY);
	void PollConnected();
	bool GuiRunning();

	void DoTimer(void);
	void CheckDro(const int index, const double value);
	void MDI(const char * string);
	void MDI(const char * string, const double value);
	void Jog(const double x, const double y, const double z,
				 const double a, const double b, const double c);
	void Toggle(CNC_IO_ID pin);
	void ControlChanged(const unsigned int index, const double value);
	int GetJogAxis(int button);
	void JogAxis(int button, bool run);
	void GetTools();
	void PopulateTool(CNC_TOOL_DATA& data, RoundTool * tool);
	void SyncTools();
	void ScanTools();
	void SendString(const int index, const wstring value);
	void EnableControls();
	void StatusText(const wstring& msg);
	void DrivesOnOff();
	bool CheckPause();
	bool WaitMove();

	bool m_step;
	double m_jogVel;
	double m_stepSize;
	int m_oldLine;
	CNC_CART_DOUBLE m_jogging;
	bool m_fastJog;
	CNC_TOOL_DATA m_tools[CNC_MAX_TOOLS];
	unsigned int m_toolCount;
    string m_activeCodes;
	bool m_connected;
	int m_connectCount;
	CURSTATE m_curState;
	double m_zMax;

	HMODULE m_cncDll;
	CNCGETTOOLDATA CncGetToolData;
	CNCISSERVERCONNECTED CncIsServerConnected;
	CNCCONNECTSERVER CncConnectServer;
	CNCDISCONNECTSERVER CncDisConnectServer;
	CNCLOADJOB CncLoadJob;
	CNCLOADTOOLTABLE CncLoadToolTable;
	CNCUPDATETOOLDATA CncUpdateToolData;
	CNCGETSTATE CncGetState;
	CNCGETCURRENTT CncGetCurrentT;
	CNCGETSTATETEXT CncGetStateText;
	CNCGETCURINTERPRETERLINE CncGetCurInterpreterLine;
	CNCGETCUREXECLINE CncGetCurExecLine;
	CNCGETJOINTSTATUS CncGetJointStatus;
	CNCGETSIMULATIONMODE CncGetSimulationMode;
	CNCGETALLAXESHOMED CncGetAllAxesHomed;
	CNCGETSINGLESTEPMODE CncGetSingleStepMode;
	CNCGETBLOCKDELETE CncGetBlockDelete;
	CNCGETACTUALFEEDOVERRIDE CncGetActualFeedOverride;
	CNCGETACTUALFEED CncGetActualFeed;
	CNCGETWORKPOS CncGetWorkPos;
	CNCGETMACHINEPOS CncGetMachinePos;
	CNCGETIOSTATUS CncGetIOStatus;
	CNCGETCURRENTMCODESTEXT CncGetCurrentMcodesText;
	CNCGETCURRENTGCODESTEXT CncGetCurrentGcodesText;
	CNCRUNSINGLELINE CncRunSingleLine;
	CNCSTARTJOG CncStartJog;
	CNCSETOUTPUT CncSetOutput;
	CNCSETFEEDOVERRIDE CncSetFeedOverride;
	CNCREWINDJOB CncRewindJob;
	CNCRESET CncReset;
	CNCRESET2 CncReset2;
	CNCGETTRACKINGSTATUS  CncGetTrackingStatus;
	CNCPAUSEJOB CncPauseJob;
	CNCRUNORRESUMEJOB CncRunOrResumeJob;
	CNCSETSIMULATIONMODE CncSetSimulationMode;
	CNCSINGLESTEPMODE CncSingleStepMode;
	CNCENABLEBLOCKDELETE CncEnableBlockDelete;
	CNCSTOPJOG CncStopJog;
	CNCGETINPUT CncGetInput;
	CNCABORTJOB CncAbortJob;
	CNCGETEMSTOPACTIVE CncGetEMStopActive;
	CNCMOVETO CncMoveTo;
	CNCGETACTUALTOOLZOFFSET CncGetActualToolZOffset;
	CNCGETACTUALORIGINOFFSET CncGetActualOriginOffset;
	CNCSETTRACKINGVELOCITY CncSetTrackingVelocity;
	CNCSTARTVELOCITYTRACKING CncStartVelocityTracking;
	CNCSTOPTRACKING CncStopTracking;
	CNCGETJOINTCONFIG CncGetJointConfig;

};

#endif

