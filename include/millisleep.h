#ifndef MILLISLEEP_H_INCLUDED
#define MILLISLEEP_H_INCLUDED

#ifdef WIN32
#include "windows.h"
#elif _POSIX_C_SOURCE >+ 199309L
#include <time.h> //for nanosleep
#else
#include <unistd.h> //for usleep
#endif
void sleep_ms(const unsigned int time)
{
#ifdef WIN32
    Sleep(time);
#elif _POSIX_C_SOURCE >+ 199309L
    struct timespec ts;
    ts.tv_sec = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(time * 1000);
#endif
}


#endif //#ifndef MILLISLEEP_H_INCLUDED

