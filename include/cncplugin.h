#ifndef CNCPLUGIN_H_INCLUDED
#define CNCPLUGIN_H_INCLUDED

#include <stdio.h>
#include <vector>
#include <string>


#if defined(_WIN32) || defined(_WIN64)
    #include "windows.h"
	typedef std::wstring CncString; //wide string on Windows
#if defined _MSC_VER && _MSC_VER < 1600 //some versions of MSVC don't include stdint
	typedef unsigned __int32 uint32_t;
#else
    #include <stdint.h>
#endif
    #define _USING_WINDOWS
    #define LIBHANDLE HMODULE
#else
	typedef std::string CncString; //UTF-8 on Linux
    #include <stdint.h>
    #define LIBHANDLE void *
#endif


#ifdef __cplusplus //DLL interface is pure C for compatibility across compilers
extern "C" {
#endif

#ifdef _CREATING_CLIENT
   #ifdef _USING_WINDOWS
      #define EXPORT_CNC __declspec(dllimport)
      #define IMPORT_CNC __declspec(dllexport)
   #else
      #define EXPORT_CNC
      #define IMPORT_CNC __attribute__ ((visibility("default")))
   #endif

	typedef uint32_t (* CNCSTARTFUNC) ();
	typedef void (* CNCSTOPFUNC) ();
    typedef const char * (* CNCGETNAMEFUNC)();
	typedef void (* CNCQUITFUNC) ();
	typedef void (* CNCPOLLFUNC) ();
	typedef uint32_t (* CNCCONTROLEXISTSFUNC) (const char * );

#else
    #ifdef _USING_WINDOWS
      #define EXPORT_CNC __declspec(dllexport)
      #define IMPORT_CNC __declspec(dllimport)
    #else
      #define EXPORT_CNC __attribute__ ((visibility("default")))
      #define IMPORT_CNC
    #endif

/*	Start the server connection. Use this for instance to start a local copy of the server.
    Return false if there is an error.
*/
    EXPORT_CNC uint32_t Start();

/*	Stop the server connection. Use this for instance to stop a local copy of the server.
*/
    EXPORT_CNC void Stop();

 /*	return a descriptive name for this plugin.
    The string should be a static pointer and should be encoded with UTF-8.
*/
    EXPORT_CNC const char * GetName();

/* Check for the existence of the controller. pluginDir is a UTF-8 encoded path to the plugin directory including a trailing slash.
    returns true if the control exists on this machine
*/
    EXPORT_CNC const uint32_t ControlExists(const char * pluginDir);

/* Called just before the library is unloaded
*/
    EXPORT_CNC void Quit();

/*Called on a (fairly) regular basis. The exact timing is dependent on the server app but should be at least every 500ms.
*/
    EXPORT_CNC void Poll();
#endif

#ifdef __cplusplus
}
#endif


#ifdef _USING_WINDOWS
/* Convert UTF8 to Windows WCHAR
*/
CncString from_utf8(const char * string);
std::string to_utf8(const CncString& string);

#else
CncString from_utf8(const char * string);
std::string to_utf8(const CncString& string);
#endif

/* Open a file with UTF-8 encoded path
*/
FILE * ufopen(const char * file, const char * mode);



#endif // CNCCLIENT_H_INCLUDED
