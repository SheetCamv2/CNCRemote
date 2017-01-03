#include "cncplugin.h"
#include "EdingCNC.h"

EdingCnc * g_cnc = NULL;

EXPORT_CNC uint32_t PLUGINAPI Start()
{
    g_cnc = new EdingCNC;
}

EXPORT_CNC const char * GetName()
{
    return ("Eding CNC V4.02");
}

EXPORT_CNC const uint32_t ControlExists()
{
    if(!g_cnc) return ctrlNONE; //should never happen
    if(!g_cnc.LoadDll()) return ctrlNONE; //dll not found
    return(ctrlLOCAL); //control exists on this computer
}

EXPORT_CNC void PLUGINAPI Quit()
{
    delete g_cnc;
    g_cnc = NULL;
}

EXPORT_CNC void PLUGINAPI Poll()
{
    g_cnc.Poll();
}
