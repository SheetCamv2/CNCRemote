//=================================================================
//    _____       _   _                    ____   _   _    ____
//   | ____|   __| | (_)  _ __     __ _   / ___| | \ | |  / ___|
//   |  _|    / _` | | | | '_ \   / _` | | |     |  \| | | |
//   | |___  | (_| | | | | | | | | (_| | | |___  | |\  | | |___
//   |_____|  \__,_| |_| |_| |_|  \__, |  \____| |_| \_|  \____|
//                                |___/
// ================================================================= 


#ifndef TOS_H
#define TOS_H

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <time.h>
#include <.\sys\types.h>
#include <.\sys\timeb.h>
#else
/* Linux */
#include <pthread.h>
#endif


#ifdef EXP2DF
	#undef EXP2DF
#endif

#ifdef CNCAPI_EXPORTS
	#define EXP2DF __declspec(dllexport)
#else
	#ifndef TOS_STATIC
		#define EXP2DF __declspec(dllimport)
	#else
		#define EXP2DF
	#endif
#endif


#define	TOS_MAX_COMMAND_LINE		260
#define TOS_MAX_ARGUMENTS           20
#define TOS_MAX_SHARED_MEM          100
#define TOS_MAX_MAILBOXES			100
#define TOS_MAX_MAILBOX_MESSAGES	255
#define TOS_MAX_SERIAL_PORTS        20

#define TOS_MAX_NAME_LENGTH			260    /* Target os max name length */
#define TOS_INDEFINITE_WAIT			0xFFFFFFFF
#define TOS_IMMEDIATE_RETURN		0
#define MSEC_PER_TICK				15
#define TOS_NO_MBXID                ((TOS_MBXID)0xFFFFFFFF)
#define TOS_NO_SEMID                ((TOS_SEMID)0xFFFFFFFF)
#define TOS_NO_MEMPTR               ((void *)0x0)
#define TOS_NO_PROCESSID            ((TOS_PROCESSID)0x0) //OxFFFFFFFF is a valid pid in windows!
#define TOS_USED					1
#define TOS_FREE					0


typedef HANDLE  TOS_EVENT_HANDLE;
typedef HANDLE  TOS_MUTEX_HANDLE;
typedef HANDLE  TOS_SEMAPHORE_HANDLE;
typedef DWORD   TOS_PROCESSID;
typedef wchar_t *TOS_PROCESS_ENTRY;  /* name of process */
typedef int     TOS_MBXID;
typedef CRITICAL_SECTION TOS_CRITICAL_SECTION; 

typedef struct TOS_THREADID
{
#ifdef _WIN32
	HANDLE m_hThread;
	DWORD  m_threadId;
#else
    pthread_t thread;
#endif

} TOS_THREADID;

typedef DWORD (__stdcall *TOS_TASK_ENTRY)(void * lpThreadParameter);


#define TOS_THREAD_PRIORITY_BELOW_NORMAL	THREAD_PRIORITY_BELOW_NORMAL
#define TOS_THREAD_PRIORITY_NORMAL			THREAD_PRIORITY_NORMAL
#define TOS_THREAD_PRIORITY_ABOVE_NORMAL	THREAD_PRIORITY_ABOVE_NORMAL
#define TOS_THREAD_PRIORITY_HIGH			THREAD_PRIORITY_HIGHEST
#define TOS_THREAD_PRIORITY_TIME_CRITICAL   THREAD_PRIORITY_TIME_CRITICAL


typedef enum
{
    TOS_NO_ERROR,
    TOS_ALREADY_EXISTING,
    TOS_TIMEOUT,
    TOS_ERROR_INTERRUPT,
    TOS_ERROR,
    TOS_ABANDONED,
    TOS_WRONG_SIZE,
    TOS_NO_MEM,
    TOS_INVALID_NAME,
    TOS_RESOURCE_FINISHED,
    TOS_MBX_FULL,
	TOS_MBX_EMPTY,
	TOS_INVALID_ID,
	TOS_MESSAGE_TOO_LONG,
} TOS_RET;

typedef enum
{
    TOS_PP_IDLE,
    TOS_PP_NORMAL,
    TOS_PP_HIGH,
    TOS_PP_REALTIME
} TOS_PROCESS_PRIORITY;

typedef enum 
{
	TOS_PM_DETACHED,
	TOS_PM_NEW_CONSOLE,
	TOS_PM_NORMAL
} TOS_PROCESS_MODE;


typedef enum 
{
	TOS_LANG_any    		= 0,
	TOS_LANG_Arabic				= 1,/*					1025	Arabic			  */
	TOS_LANG_Basque				= 2,/*					1069	ANSI			  */
	TOS_LANG_Catalan			= 3,/*					1027	ANSI			  */
	TOS_LANG_Chinese_s			= 4,/*	(Simplified)	2052	GB2312			  */
	TOS_LANG_Chinese_t			= 5,/*	(Traditional)	1028	Chinese-Big 5	  */
	TOS_LANG_Czech				= 6,/*					1029	Eastern European  */
	TOS_LANG_Danish				= 7,/*					1030	ANSI			  */
	TOS_LANG_Dutch				= 8,/*					1043	ANSI			  */
	TOS_LANG_English_us			= 9,/*					1033	ANSI			  */
	TOS_LANG_Finnish			= 10,/*					1035	ANSI			  */
	TOS_LANG_French				= 11,/*					1036	ANSI			  */
	TOS_LANG_German				= 12,/*					1031	ANSI			  */
	TOS_LANG_Greek				= 13,/*					1032	Greek			  */
	TOS_LANG_Hebrew				= 14,/*					1037	Hebrew			  */
	TOS_LANG_Hungarian			= 15,/*					1038	Eastern European  */
	TOS_LANG_Italian			= 16,/*					1040	ANSI			  */
	TOS_LANG_Japanese			= 17,/*					1041	Shift-JIS		  */
	TOS_LANG_Korean				= 18,/*					1042	Johab			  */
	TOS_LANG_Norwegian			= 19,/*					1044	ANSI			  */
	TOS_LANG_Polish				= 20,/*					1045	Eastern European  */
	TOS_LANG_Portuguese			= 21,/*					2070	ANSI			  */
	TOS_LANG_Portuguese_Brazil  = 22,/*					1046	ANSI			  */
	TOS_LANG_Russian			= 23,/*					1049	Russian			  */
	TOS_LANG_Slovakian			= 24,/*					1051	Eastern European  */
	TOS_LANG_Slovenian			= 25,/*					1060	Eastern European  */
	TOS_LANG_Spanish			= 26,/*					3082	ANSI			  */
	TOS_LANG_Swedish			= 27,/*					1053	ANSI			  */
	TOS_LANG_Turkish			= 28,/*					1055	Turkish			  */
	TOS_LANG_LAST			    = 30

} TOS_LANG_ID;


//hi res timer
typedef struct _TOS_TIME_STAMP
{
#ifdef _WIN32
    LONGLONG startTime; 
    LONGLONG timeStamp;
#else
    /* LINUX */
    timeval startTime;                         
    timeval timeStamp;
#endif

} TOS_TIME_STAMP;

#ifdef __cplusplus
extern "C" {
#endif

/////////////////
//TosInit should be called before any other Tos functionality
//TosTerm when the process ends
//////////////////
extern TOS_RET		EXP2DF __stdcall TosInit(void);
extern TOS_RET		EXP2DF __stdcall TosTerm(void);

//////////////////
//Critical sections, fast mutex within 1 process
//////////////////
extern TOS_RET EXP2DF __stdcall TosOpenCriticalSection (TOS_CRITICAL_SECTION *pSection);
extern TOS_RET EXP2DF __stdcall TosCloseCriticalSection (TOS_CRITICAL_SECTION *pSection);
extern TOS_RET EXP2DF __stdcall TosEnterCriticalSection (TOS_CRITICAL_SECTION *pSection);
extern TOS_RET EXP2DF __stdcall TosLeaveCriticalSection (TOS_CRITICAL_SECTION *pSection);

/////////////////
//Mutex, use NULL for unnamed, interprocess capability for named MUTEX
/////////////////
extern TOS_RET		EXP2DF __stdcall TosOpenMutex (char *name, TOS_MUTEX_HANDLE *handle);
extern TOS_RET		EXP2DF __stdcall TosCloseMutex (TOS_MUTEX_HANDLE handle);
extern TOS_RET      EXP2DF __stdcall TosRequestMutex (TOS_MUTEX_HANDLE handle,unsigned int timeOut);
extern TOS_RET		EXP2DF __stdcall TosReleaseMutex (TOS_MUTEX_HANDLE handle);

/////////////////
//Events, default not set
/////////////////
extern TOS_RET		EXP2DF __stdcall TosOpenEvent (char *name, TOS_EVENT_HANDLE *handle);
extern TOS_RET		EXP2DF __stdcall TosCloseEvent (TOS_EVENT_HANDLE handle);
extern TOS_RET      EXP2DF __stdcall TosWaitEvent (TOS_EVENT_HANDLE handle, unsigned int timeOut);
extern TOS_RET		EXP2DF __stdcall TosSetEvent (TOS_EVENT_HANDLE handle);
extern TOS_RET		EXP2DF __stdcall TosResetEvent (TOS_EVENT_HANDLE handle);


/////////////////
//Counting semaphores, use NULL for unnamed semaphores
/////////////////
extern TOS_RET		EXP2DF __stdcall TosOpenSemaphore (char *name, int count, TOS_SEMAPHORE_HANDLE *handle);
extern TOS_RET		EXP2DF __stdcall TosCloseSemaphore (TOS_SEMAPHORE_HANDLE handle);
extern TOS_RET      EXP2DF __stdcall TosRequestSemaphore (TOS_SEMAPHORE_HANDLE handle,unsigned int timeOut);
extern TOS_RET		EXP2DF __stdcall TosReleaseSemaphore (TOS_SEMAPHORE_HANDLE handle);

//Named shared memory
extern TOS_RET      EXP2DF __stdcall TosOpenSharedMemory (void **ppvoidSharedMemory, unsigned int size, char *name);
extern TOS_RET		EXP2DF __stdcall TosCloseSharedMemory (void *pvoidSharedMemory);

/////////////////////////////////////
//Mailboxes, inter process capability
//Uses tos shared memory, Tos Mutex and Tos Event
/////////////////////////////////////
extern TOS_RET      EXP2DF __stdcall TosOpenMbx (char *pszMbxName, unsigned int ulSize, TOS_MBXID *pMbxId);
extern TOS_RET		EXP2DF __stdcall TosCloseMbx (TOS_MBXID mbxId);
extern TOS_RET      EXP2DF __stdcall TosReadMbx (TOS_MBXID mbxId, unsigned int uMaxMsgSize, double dTimeOutPeriod, char *pszMessage, unsigned int *puActualMsgSize);
extern TOS_RET EXP2DF __stdcall TosWriteMbx (TOS_MBXID mbxId, char *pszMessage, unsigned int uMsgSize);
///////////////////////////
//Thread functionality
//////////////////////////
extern TOS_RET		EXP2DF __stdcall TosCreateThread(TOS_TASK_ENTRY taskEntry, void *taskParameter, TOS_THREADID *pThreadId);
extern void			EXP2DF __stdcall TosSetThreadPriority(TOS_THREADID *pTrhreadId, int priority);
extern int			EXP2DF __stdcall TosGetThreadPriority(TOS_THREADID *pTrhreadId);
extern TOS_THREADID EXP2DF __stdcall TosGetCurrentThreadID(void);


////////////////////////
//Process functions
////////////////////////
extern TOS_PROCESSID EXP2DF __stdcall TosGetCurrentProcessId (void);
extern TOS_RET		 EXP2DF __stdcall TosCreateProcess (TOS_PROCESS_ENTRY vosProcessEntry, wchar_t commandLine[], TOS_PROCESS_MODE mode, TOS_PROCESS_PRIORITY priority, TOS_PROCESSID *pProcessId);
extern TOS_RET		 EXP2DF __stdcall TosDeleteProcess (TOS_PROCESSID tosProcessId);
extern TOS_RET		 EXP2DF __stdcall TosGetProcessId (TOS_PROCESSID *pProcessId);
extern TOS_RET EXP2DF __stdcall TosWaitProcess (TOS_PROCESSID tosProcessId, unsigned int uTimeOut, int *pExitCode);
extern TOS_RET		 EXP2DF __stdcall TosSetProcessPriority (TOS_PROCESSID processId, TOS_PROCESS_PRIORITY priority);
extern TOS_RET		 EXP2DF __stdcall TosGetProcessPriority (TOS_PROCESSID processId, TOS_PROCESS_PRIORITY *priority);
extern int			 EXP2DF __stdcall TosIsElevatedAdministrator (HANDLE hInputToken); 


//Sleep milliseconds
extern TOS_RET EXP2DF __stdcall TosSleep (unsigned int millisecs);

////////////////////////
//Hi res timer functions
////////////////////////
extern void   EXP2DF __stdcall TosStartTimer(TOS_TIME_STAMP *pTimeStamp);
extern double EXP2DF __stdcall TosStopTimer(TOS_TIME_STAMP *timeStamp);
extern double EXP2DF __stdcall TosTimerElapse(TOS_TIME_STAMP *timeStamp);
extern int    EXP2DF __stdcall TosIsTimerExpired(TOS_TIME_STAMP *timeStamp, double duration, double *pSecondsSinceStart);

////////////////////////////////////////////////
//Serial port communication (RS232 or USB-CDC)
////////////////////////////////////////////////
//Open serial port
extern TOS_RET EXP2DF __stdcall TosSerialPortInit( int portNumber, int baudRate, unsigned char dataBits, unsigned char stopBits, char parity );
//Close serial port
extern void EXP2DF __stdcall TosSerialPortClose ( int portNumber );
//Flush all buffers
extern void EXP2DF __stdcall TosSerialPortReset(int portNumber);
//Set read timeout
extern void EXP2DF __stdcall TosSerialPortSetReadTimeout( int portNumber, unsigned int ms );
//Read msg, message complete after timeout after last char
extern int EXP2DF __stdcall TosSerialPortReadMsg(int portNumber, unsigned char *pData, unsigned int maxLen);
//Write msg
extern int EXP2DF __stdcall TosSerialPortWrite(int portNumber, const unsigned char *pData, unsigned int len); 


///////////////////////
//Get locale of system
///////////////////////
extern int 			EXP2DF __stdcall TosGetLocale(void);
extern TOS_LANG_ID  EXP2DF __stdcall TosGetLanguageID(void);
extern char         EXP2DF * __stdcall TosGetLanguageText(TOS_LANG_ID id);


#ifdef __cplusplus
}
#endif


#endif //defined TOS_H

