#include "main.h"
#include "sim.h"
#include "millisleep.h"

int main(int argc, char * argv[])
{
    Sim sim;
    if(sim.Connect("tcp://*:" DEFAULT_COMMS_PORT) != CncRemote::Comms::errOK)
    {
        printf("Failed to bind. Is another instance of the server running?\n");
        return -1;
    }

    while(1)
    {
        sim.Poll();
        sleep_ms(1);
    }
    return 0;
}
