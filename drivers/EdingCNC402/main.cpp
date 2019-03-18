/****************************************************************
EdingCNC server plugin
Copyright 2018 Stable Design <les@sheetcam.com>


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
#include "winsock2.h"
#include "cncplugin.h"
#include "cncapi2.h"
#include "EdingCNC.h"

EdingCncServer * g_cnc = NULL;

EXPORT_CNC uint32_t Start()
{
	if(!g_cnc) 	g_cnc = new EdingCncServer;
	return g_cnc->Bind() == errOK;
}

EXPORT_CNC void Stop()
{
}

EXPORT_CNC const char * GetName()
{
#ifdef _DEBUG
	return ("Eding CNC (debug)");
#else
	return ("Eding CNC");
#endif
}

EXPORT_CNC const uint32_t ControlExists(const char * pluginDir)
{
    if(!g_cnc) 	g_cnc = new EdingCncServer;
	return(g_cnc->LoadDll());
}

EXPORT_CNC void Quit()
{
    delete g_cnc;
    g_cnc = NULL;
}

EXPORT_CNC void Poll()
{
    g_cnc->Poll();
}
