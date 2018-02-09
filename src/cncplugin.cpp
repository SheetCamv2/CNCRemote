/****************************************************************
CNCRemote plugin helper
Copyright 2017 Stable Design <les@sheetcam.com>


This program is free software; you can redistribute it and/or modify
it under the terms of the Mozilla Public License Version 2.0 or later
as published by the Mozilla foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License for more details.

You should have received a copy of the Mozilla Public License
along with this program; if not, you can obtain a copy from mozilla.org
******************************************************************/


#include "cncplugin.h"
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef _WIN32
#include "Shlwapi.h"
using namespace std;

wstring from_utf8(const char * string)
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

string to_utf8(const CncString& string)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, string.c_str(), string.length(),
                                  0, 0, 0, 0);
    if (!len)
        return "ErrorA";

    std::vector<char> abuff(len + 1);

    // NOTE: this does not NULL terminate the string in abuff, but this is ok
    //       since it was zero-initialized in the vector constructor
    if (!WideCharToMultiByte(CP_UTF8, 0, string.c_str(), string.length(),
                             &abuff[0], len, 0, 0))
    {
        return "ErrorA";
    }//if

    return &abuff[0];
}
#else
CncString from_utf8(const char * string)
{
    return CncString(string);
}

std::string to_utf8(const CncString& string)
{
    return(string);
}

#endif



FILE * ufopen(const char * file, const char * mode)
{
#ifdef _WIN32
    FILE * ret = _wfopen(from_utf8(file).c_str(), from_utf8(mode).c_str());
    return ret;
#else
    return fopen(file, mode);
#endif
}

