#include "main.h"
#include "sim.h"

int main(int argc, char * argv[])
{
    Sim sim;
    if(sim.Bind() != CncRemote::errOK)
    {
        printf("Failed to bind port. Is another instance of the server running?\n");
        return -1;
    }

    while(1)
    {
        sim.Poll();
#ifdef _WIN32
        Sleep(1);
#else
        sleep_ms(1);
#endif
    }
    return 0;
}
