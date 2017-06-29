#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <time.h>
#include <string>
#include "pstdint.h"


using namespace std;

//Sleep for a given time in milliseconds
void SleepMs(const unsigned int time);


/* microsecond timer*/
class UTimer
{
public:
    UTimer();
    void Restart(); //Restart the timer
    uint64_t GetElapsed(const bool restart = false); //get elapsed time in us

private:
#ifndef _LINUX
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_time;
#else
    timespec m_time;
#endif
};


//used for debug timing
class TestTimer : public UTimer
{
public:
    TestTimer(const string& msg); //Show this message along with the time


    void Check(); //If elapsed time since the last call to Check has changed display it.

private:
	unsigned long m_lastTime;
    string m_msg;
};

#endif//#ifndef DEBUG_TIMER_H_INCLUDED
