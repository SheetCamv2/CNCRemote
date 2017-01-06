#ifndef DEBUG_TIMER_H_INCLUDED
#define DEBUG_TIMER_H_INCLUDED

#include <time.h>

class TestTimer
{
public:
    TestTimer(const string& msg)
    {
        m_msg = msg;
        clock_gettime(CLOCK_MONOTONIC, &m_time);
        m_lastTime = 0;
    }

    void Start()
    {
        clock_gettime(CLOCK_MONOTONIC, &m_time);
    }

    void Check()
    {
        timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        unsigned long diff = time.tv_nsec - m_time.tv_nsec;
        if(diff > m_lastTime)
        {
            m_lastTime = diff;
            printf("%s = %f ms\n", m_msg.c_str(), (float)diff / 1000000);
        }
    }

private:
    timespec m_time;
    unsigned long m_lastTime;
    string m_msg;
};

#endif//#ifndef DEBUG_TIMER_H_INCLUDED
