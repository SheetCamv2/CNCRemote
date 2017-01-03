#include "cncplugin.h"
#include "time.h"
//this is about the simplest plugin possible.

EXPORT_CNC uint32_t Start()
{
    return true; //we don't need to initialize anything
}

EXPORT_CNC const char * GetName()
{
    return ("Simulator");
}

EXPORT_CNC const uint32_t ControlExists()
{
    return(ctrlREMOTE); //this could be running on a remote machine so we don't know if it exists
}

EXPORT_CNC void Quit()
{
}

EXPORT_CNC void Poll()
{

}
