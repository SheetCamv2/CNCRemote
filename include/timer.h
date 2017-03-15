#ifndef DEBUG_TIMER_H_INCLUDED
#define DEBUG_TIMER_H_INCLUDED

#include <time.h>

class TestTimer
{
public:
    TestTimer(const string& msg)
    {
		m_msg = msg;
#ifdef _USING_WINDOWS
		m_time = timeGetTime();
#else
        clock_gettime(CLOCK_MONOTONIC, &m_time);
#endif
		m_lastTime = 0;
    }

    void Start()
    {
#ifdef _USING_WINDOWS
		m_time = timeGetTime();
#else
        clock_gettime(CLOCK_MONOTONIC, &m_time);
#endif
	}

    void Check()
    {
#ifdef _USING_WINDOWS
		unsigned long diff = 0;
		DWORD time = timeGetTime();
		diff = time - m_time;
#else
        timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        unsigned long diff = time.tv_nsec - m_time.tv_nsec;
#endif
		if(diff > m_lastTime)
        {
            m_lastTime = diff;
            printf("%s = %f ms\n", m_msg.c_str(), (float)diff / 1000000);
        }
    }

private:
#ifdef _USING_WINDOWS
	DWORD m_time;
#else
    timespec m_time;
#endif
	unsigned long m_lastTime;
    string m_msg;
};

#endif//#ifndef DEBUG_TIMER_H_INCLUDED
