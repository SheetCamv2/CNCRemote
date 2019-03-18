#pragma once
#include "cncserver.h"
#include "invhelp.h"
#include <windowsx.h>

using namespace std;
using namespace CncRemote;


class MachServer : public CncRemote::Server
{
public:
	MachServer();
	~MachServer();
	void StartLoop();
	virtual void Poll();

protected:
	virtual void UpdateState(State& state);
	virtual void DrivesOn(const bool state);
	virtual void JogVel(const Axes velocities);
	virtual void JogStep(const Axes distance, const double speed);
	virtual bool Mdi(const string line);
	virtual void SpindleOverride(const double percent);
	virtual void FeedOverride(const double percent);
	virtual void RapidOverride(const double percent);
	virtual bool LoadFile(const string file);
	virtual bool CloseFile();
	virtual void CycleStart();
	virtual void CycleStop();
	virtual void FeedHold(const bool state);
	virtual void BlockDelete(const bool state);
	virtual void SingleStep(const bool state);
	virtual void OptionalStop(const bool state);
	virtual void Home(const BoolAxes axes);
	virtual Axes GetOffset(const unsigned int index);
	virtual vector<int> GetGCodes();
	virtual vector<int> GetMCodes();




	struct AXISDATA
	{
		double offset;
		double scale;
		int targetJog;
		bool jogging;
		int acc;
		int index;
		double homeSpeed;
	};

	AXISDATA m_jogAxes[MAX_AXES];

protected:
private:
	static void LoadThread(void * param);
	bool LoadSettings();

	bool running;
	OLECHAR m_filePath[MAX_PATH];

	LPDISPATCH m_Mach4App;
	LPDISPATCH m_Mach4Scripter;
};