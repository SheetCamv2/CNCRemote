#ifndef DEBUG_TIMER_H_INCLUDED
#define DEBUG_TIMER_H_INCLUDED

#include <time.h>

/* microsecond timer*/
class UTimer
{
public:
    UTimer()
    {
#ifdef _USING_WINDOWS
        QueryPerformanceFrequency(&m_frequency);
#endif
        Restart();
    }

    void Restart()
    {
#ifdef _USING_WINDOWS
		QueryPerformanceCounter(&m_time);
#else
        clock_gettime(CLOCK_MONOTONIC, &m_time);
#endif
	}

    uint64_t GetElapsed(const bool restart = false)
    {
#ifdef _USING_WINDOWS
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		LARGE_INTEGER ret;
		ret.QuadPart = now.QuadPart - m_time.QuadPart;
		ret.QuadPart *= 1000000;
        ret.QuadPart /= m_frequency.QuadPart;
        if(restart) m_time = now;
        return ret.QuadPart;
#else
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t t1 = (now.tv_nsec / 1000) + (now.tv_sec * 1000000);
        uint64_t t2 = (m_time.tv_nsec / 1000) + (m_time.tv_sec * 1000000);
        if(restart) m_time = now;
        return (t1 - t2);
#endif

    }

private:
#ifdef _USING_WINDOWS
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_time;
#else
    timespec m_time;
#endif
};


class TestTimer : public UTimer
{
public:
    TestTimer(const string& msg)
    {
		m_msg = msg;
		m_lastTime = 0;
    }


    void Check()
    {
        uint64_t diff = GetElapsed(true);
        if(diff > 200)
        {
            m_lastTime = diff;
            printf("%s = %f ms\n", m_msg.c_str(), (float)diff / 1000);
        }
    }

private:
	unsigned long m_lastTime;
    string m_msg;
};

#endif//#ifndef DEBUG_TIMER_H_INCLUDED
