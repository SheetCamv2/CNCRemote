#include "main.h"
#include "sim.h"

int main(int argc, char * argv[])
{
    Sim sim;
	puts("Simulator started");
    if(sim.Bind() != CncRemote::errOK)
    {
        puts("Failed to bind port. Is another instance of the server running?\n");
        return -1;
    }


    while(1)
    {
        sim.Poll();
#ifdef _WIN32
        Sleep(20);
#else
        SleepMs(1);
#endif
    }
    return 0;
}
