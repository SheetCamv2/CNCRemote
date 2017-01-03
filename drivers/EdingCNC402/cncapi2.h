#ifndef CNCAPI_2_H_INCLUDED
#define CNCAPI_2_H_INCLUDED
#include "cnc_types.h"
#include "CncLang.h"
#include "Tos.h"
#include "utl.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef CNC_PAUSE_STS* (__stdcall * CNCGETPAUSESTATUS)(void);
typedef CNC_SPINDLE_STS* (__stdcall * CNCGETSPINDLESTATUS)(void);
typedef CNC_RC (__stdcall *CNCSETTRACKINGVELOCITY)(CNC_CART_DOUBLE vel, 
												CNC_CART_BOOL   move);
typedef CNC_RC (__stdcall *CNCSTARTVELOCITYTRACKING)(void);
typedef CNC_RC (__stdcall *CNCSTOPTRACKING)(void);
typedef CNC_JOINT_CFG* (__stdcall *CNCGETJOINTCONFIG)(int joint);



typedef CNC_TOOL_DATA (__stdcall * CNCGETTOOLDATA)(int index);
typedef int (__stdcall * CNCISSERVERCONNECTED)(void);
typedef CNC_RC (__stdcall * CNCCONNECTSERVER)(char *iniFileName);
typedef CNC_RC (__stdcall * CNCDISCONNECTSERVER)(void);
typedef CNC_RC (__stdcall * CNCLOADJOB)(const wchar_t *fileName);
/*typedef CNC_RC (__stdcall * CNCLOADJOB)(const wchar_t *fileName, 
	 int        *jobFileLength, 
	 int        *macroFileLength,
	 int        *isLongJob,
	 int        *isSuperLongJob,
	 __int64    *sizeInBytes);*/
typedef CNC_RC (__stdcall * CNCLOADTOOLTABLE)(void);
typedef CNC_RC (__stdcall * CNCUPDATETOOLDATA)(CNC_TOOL_DATA *pTool, int index);
typedef CNC_IE_STATE (__stdcall * CNCGETSTATE)(void);
typedef int (__stdcall * CNCGETCURRENTT)(void);
typedef char* ( __stdcall * CNCGETSTATETEXT)(int state);
typedef int (__stdcall * CNCGETCURINTERPRETERLINE)(void);
typedef int (__stdcall * CNCGETCUREXECLINE)(void);
typedef CNC_JOINT_STS* (__stdcall * CNCGETJOINTSTATUS)(int jointIndex);
typedef int (__stdcall * CNCGETSIMULATIONMODE)(void);
typedef int  (__stdcall * CNCGETALLAXESHOMED)(void);
typedef int (__stdcall * CNCGETSINGLESTEPMODE)(void);
typedef int (__stdcall * CNCGETBLOCKDELETE)(void);
typedef double (__stdcall * CNCGETACTUALFEEDOVERRIDE)(void);
typedef double (__stdcall * CNCGETACTUALFEED)(void);
typedef CNC_CART_DOUBLE (__stdcall * CNCGETWORKPOS)(void);
typedef CNC_CART_DOUBLE (__stdcall * CNCGETMACHINEPOS)(void);
typedef CNC_IO_PORT_STS* (__stdcall * CNCGETIOSTATUS)(CNC_IO_ID id);
typedef void (__stdcall * CNCGETCURRENTMCODESTEXT)(char *activeMCodes);
typedef void (__stdcall * CNCGETCURRENTGCODESTEXT)(char *activeGCodes);
typedef CNC_RC (__stdcall * CNCRUNSINGLELINE)(char *text);
typedef CNC_RC (__stdcall * CNCSTARTJOG)(double axis[], 
									 double velocityFactor, 
									 int continuous);
typedef CNC_RC (__stdcall * CNCSETOUTPUT)(CNC_IO_ID id, int value);
typedef CNC_RC (__stdcall * CNCSETFEEDOVERRIDE)(double factor);
typedef CNC_RC (__stdcall * CNCREWINDJOB)(void);
typedef CNC_RC (__stdcall * CNCRESET)(void);
typedef CNC_RC (__stdcall * CNCRESET2)(unsigned int resetFlags);
typedef CNC_TRACKING_STATUS * (__stdcall * CNCGETTRACKINGSTATUS)(void);
typedef CNC_RC (__stdcall * CNCPAUSEJOB)(void);
typedef CNC_RC (__stdcall * CNCRUNORRESUMEJOB)(void);
typedef CNC_RC (__stdcall * CNCSETSIMULATIONMODE)(int sim);
typedef CNC_RC (__stdcall * CNCSINGLESTEPMODE)(int singleStepMode);
typedef CNC_RC (__stdcall * CNCENABLEBLOCKDELETE)(int enable);
typedef CNC_RC (__stdcall * CNCSTOPJOG)(int axis);
typedef int (__stdcall * CNCGETINPUT)(CNC_IO_ID);
typedef CNC_RC (__stdcall * CNCABORTJOB)(void);
typedef int (__stdcall * CNCGETEMSTOPACTIVE)(void);
typedef CNC_RC (__stdcall * CNCMOVETO)(CNC_CART_DOUBLE pos, 
	                               CNC_CART_BOOL enable, 
								   double velocityFactor);
typedef double (__stdcall * CNCGETACTUALTOOLZOFFSET)(void);
typedef CNC_CART_DOUBLE (__stdcall * CNCGETACTUALORIGINOFFSET)(void);


#ifdef __cplusplus
	}
#endif
#endif //#ifndef CNCAPI_2_H_INCLUDED
