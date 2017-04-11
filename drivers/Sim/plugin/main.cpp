#include "cncplugin.h"
#include "time.h"
//this is about the simplest plugin possible.

EXPORT_CNC uint32_t Start()
{
    return true; //we don't need to initialize anything
}

EXPORT_CNC void Stop()
{
}

EXPORT_CNC const char * GetName()
{
#ifdef _DEBUG
    return ("Simulator (debug)");
#else
    return ("Simulator");
#endif
}

EXPORT_CNC const uint32_t ControlExists(const char * pluginDir)
{
    return(true);
}

EXPORT_CNC void Quit()
{
}

EXPORT_CNC void Poll()
{

}
