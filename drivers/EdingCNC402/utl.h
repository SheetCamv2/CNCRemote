#ifndef UTL_H_INCLUDED
#define UTL_H_INCLUDED

#include <stdio.h>
#include "cnc_types.h"


#ifdef EXP2DF
#undef EXP2DF
#endif

/* This lib is linked in cncapi.dll */

#ifdef CNCAPI_EXPORTS
#define EXP2DF __declspec(dllexport)
#else
#define EXP2DF __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

//Initialize-Terminate helper module,
void EXP2DF __stdcall UtlInit(void);
void EXP2DF __stdcall UtlTerm(void);

//Logging macro
#define UTL_LOG(functionName, errorClass, code) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0,  NULL) 

#define UTL_LOG_P0(functionName, errorClass, code, fmt) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt)

#define UTL_LOG_P1(functionName, errorClass, code, fmt, p1) \
    /*lint -e534*/UtlLog(false, functionName,  __FILE__, __LINE__, errorClass, code, 0, fmt, p1)

#define UTL_LOG_P2(functionName, errorClass, code, fmt, p1, p2) \
    /*lint -e534*/UtlLog(false, functionName,  __FILE__, __LINE__, errorClass, code, 0, fmt, p1, p2)

#define UTL_LOG_P3(functionName, errorClass, code, fmt, p1, p2, p3) \
    /*lint -e534*/UtlLog(false, functionName,  __FILE__, __LINE__, errorClass, code, 0, fmt, p1, p2, p3)

#define UTL_LOG_P4(functionName, errorClass, code, fmt, p1, p2, p3, p4) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt, p1, p2, p3, p4)

#define UTL_LOG_P5(functionName, errorClass, code, fmt, p1, p2, p3, p4, p5) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt, p1, p2, p3, p4, p5)

#define UTL_LOG_P6(functionName, errorClass, code, fmt, p1, p2, p3, p4, p5, p6) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt, p1, p2, p3, p4, p5, p6)

#define UTL_LOG_VA(functionName, errorClass, code, fmt, ...) \
    /*lint -e534*/UtlLog(false, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt, __VA_ARGS__)

#define UTL_LOG_VA_FO(functionName, errorClass, code, fmt, ...) \
    /*lint -e534*/UtlLog(true, functionName, __FILE__, __LINE__, errorClass, code, 0, fmt, __VA_ARGS__)


//Tracing macros, the higher the level, the more detail information
#ifndef TSL_SKIP_TRACING


#define UTL_TL_MAIN 0x1
#define UTL_TL_CMD  0x2
#define UTL_TL_INT  0x4
#define UTL_TL_EXE  0x8
#define UTL_TL_TRG  0x10
#define UTL_TL_TRG1 0x20
#define UTL_TL_TRG2 0x40
#define UTL_TL_TRG3 0x80


#define UTL_TRACE(functionName, l) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, NULL); }

#define UTL_TRACE0(functionName, l, fmt) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt); }

#define UTL_TRACE1(functionName, l, fmt, p1) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1); }

#define UTL_TRACE2(functionName, l, fmt, p1, p2) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2); }

#define UTL_TRACE3(functionName, l, fmt, p1, p2, p3) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3); }

#define UTL_TRACE4(functionName, l, fmt, p1, p2, p3, p4) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4); }

#define UTL_TRACE5(functionName, l, fmt, p1, p2, p3, p4, p5) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4, p5); }

#define UTL_TRACE6(functionName, l, fmt, p1, p2, p3, p4, p5, p6) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4, p5, p6); }

#define UTL_TRACE7(functionName, l, fmt, p1, p2, p3, p4, p5, p6, p7) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4, p5, p6, p7); }

#define UTL_TRACE8(functionName, l, fmt, p1, p2, p3, p4, p5, p6, p7, p8) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4, p5, p6, p7, p8); }

#define UTL_TRACE9(functionName, l, fmt, p1, p2, p3, p4, p5, p6, p7, p8, p9) {\
    UtlTrace(functionName, __FILE__, __LINE__, l, fmt, p1, p2, p3, p4, p5, p6, p7, p8, p9); }

#define UTL_TRACE_VA(functionName, l, fmt, ...) \
    /*lint -e534*/UtlTrace(functionName, __FILE__, __LINE__, l, fmt, __VA_ARGS__)

#else

/* Set macros to empty if TSL_SKIP_TRACEGING is defined */

#define UTL_TRACE0(functionName, fmt)
#define UTL_TRACE1(functionName, fmt, p1)
#define UTL_TRACE2(functionName, fmt, p1, p2)
#define UTL_TRACE3(functionName, fmt, p1, p2, p3)
#define UTL_TRACE4(functionName, fmt, p1, p2, p3, p4)
#define UTL_TRACE5(functionName, fmt, p1, p2, p3, p4, p5)
#define UTL_TRACE6(functionName, fmt, p1, p2, p3, p4, p5, p6)
#define UTL_TRACE7(functionName, fmt, p1, p2, p3, p4, p5, p6, p7)
#define UTL_TRACE8(functionName, fmt, p1, p2, p3, p4, p5, p6, p7, p8)
#define UTL_TRACE9(functionName, fmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)

#endif



/* Assert macro, remain looping until assert function returns 0 */
#define UTL_ASSERT(exp) {UtlAssert((exp), #exp, __FILE__, __LINE__)/*lint -i 722*/;}

/* Assert */
int EXP2DF __stdcall UtlAssert(int expression, void *expressionText, void *file, unsigned line);

/* Log infos warnings errors for user */
/* actual loggin decoupled to a thread, so that this can be used in the real time motion part */
void UtlLog(bool            fileOnly,       /* log to file only and not to gui */
            char            *functionName,  /* name of the function that caused the error */
            char            *file,          /* the c-file name where the code resides     */
            int             line,           /* the offending line number                  */
            CNC_ERROR_CLASS  errorClass,    /* PS error class                            */
            CNC_RC           error,         /* PS error code                             */
            int             subCode,        /* error subcode                              */
            char            *fmt, ...);     /* optional text aruments                     */

/* same fucion, however not decoupled, writein to file and use buffer happens in calling context */
void UtlLogDirect(bool  fileOnly,       /* log to file only and not to gui */
    char            *functionName,  /* name of the function that caused the error */
    char            *file,          /* the c-file name where the code resides     */
    int             line,           /* the offending line number                  */
    CNC_ERROR_CLASS  errorClass,    /* PS error class                            */
    CNC_RC           error,         /* PS error code                             */
    int             subCode,        /* error subcode                              */
    char            *fmt, ...);     /* optional text aruments                     */



/* wait until logging completed */
void UtilFlushLogging(void);

/* Trace functions */
void EXP2DF __stdcall UtlTrace(char   *functionName, /* name of the function that caused the error */
              char   *file,         /* the c-file name where the code resides     */
              int    line,          /* the offending line number                  */
              int    level,
              char   *fmt, ...);    /* optional text aruments                     */

void EXP2DF __stdcall UtlOpenTraceFile(void);
void EXP2DF __stdcall UtlCloseTraceFile(void);
void EXP2DF __stdcall UtlTraceFile( bool includeTimeStamp, char *fmt, ...);
void EXP2DF __stdcall UtlFlushTraceFile(void);
void EXP2DF __stdcall UtlSetTraceLevel(int level);



/* Read parameter from INI file and log eventual errors */
FILE   EXP2DF * __stdcall UtlIniOpenFile(char *fileName, char *openAttributes);
int    EXP2DF __stdcall UtlIniCloseFile(FILE *fp);
void   EXP2DF __stdcall UtlIniSetLogging(int on);
int    EXP2DF __stdcall UtlIniGetLogging(void);
CNC_RC EXP2DF __stdcall UtlIniReadRealParameter(char * fileName, FILE *fp, char *parameter, char *section, double *parValue);
CNC_RC EXP2DF __stdcall UtlIniReadStringParameter(char * fileName, FILE *fp, char *parameter, char *section, char *parValue, int maxLen);
CNC_RC EXP2DF __stdcall UtlIniReadIntParameter(char * fileName, FILE *fp, char *parameter, char *section, int  *parValue);
CNC_RC EXP2DF __stdcall UtlIniReadLongLongParameter(char * fileName, FILE *fp, char *parameter, char *section, int64_t  *parValue);
CNC_RC EXP2DF __stdcall UtlIniReadBoolParameter(char * fileName, FILE *fp, char *parameter, char *section, int *parValue);

/* format e.g. 0x32 to 0011 0010 */
void EXP2DF __stdcall UtlFormatBinary8(unsigned char x, char *out_10_bytes);
void EXP2DF __stdcall UtlFormatBinary16(unsigned short x, char *out_20_bytes);
void EXP2DF __stdcall UtlFormatBinary32(unsigned int x, char *out_40_bytes);
void EXP2DF __stdcall UtlFormatBinary10(unsigned short x, char *out_13_bytes);



/* Remove path from filename */
char EXP2DF * __stdcall UtlStripName( char *fileName );
wchar_t EXP2DF * __stdcall UtlStripNameU( wchar_t *fileName );

/* Case insensitive string compare */
int EXP2DF __stdcall UtlStriCmp(const char *anyCase, const char *lowerCase);

/* Case insensitive string compare */
int EXP2DF __stdcall UtlStrniCmp(const char *anyCase, const char *lowerCase, size_t count);

char EXP2DF * __stdcall UtlCreateDateTimeString( char createString[] /* take str length is 20 */);



/* rotate 2D */
void EXP2DF __stdcall UtlRotateScale(int reverse, double rotationDegrees, CNC_VECTOR rotationPoint,  CNC_VECTOR scaleFactor, double *x, double *y);

/* Convert interpreter-work position to machine position */
CNC_CART_DOUBLE EXP2DF __stdcall UtlMachinePositionFromWorkPosition(CNC_CART_DOUBLE  workPos, CNC_OFFSET_AND_PLANE op);

/* Convert machine position to interpreter-work position */
CNC_CART_DOUBLE EXP2DF __stdcall UtlWorkPositionFromMachinePosition(CNC_CART_DOUBLE machinePos, CNC_OFFSET_AND_PLANE op);




#ifdef __cplusplus
}
#endif


#endif //UTL_H_INCLUDED

