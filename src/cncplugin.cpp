#include "cncplugin.h"
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef _USING_WINDOWS
#include "Shlwapi.h"
using namespace std;

wstring utf8towchar(const char * string)
{
    int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, string, -1, NULL, 0);
    if (!len)
        return L"ErrorA2W";
    
    std::vector<wchar_t> wbuff(len);
    // NOTE: this does not NULL terminate the string in wbuff, but this is ok
    //       since it was zero-initialized in the vector constructor
    if (!MultiByteToWideChar(CP_UTF8, 0, string, -1, &wbuff[0], len))
        return L"ErrorA2W";

    return &wbuff[0];}
#endif



FILE * ufopen(const char * file, const char * mode)
{
#ifdef _USING_WINDOWS
    FILE * ret = _wfopen(utf8towchar(file).c_str(), utf8towchar(mode).c_str());
    return ret;
#else
    return fopen(file, mode);
#endif
}

