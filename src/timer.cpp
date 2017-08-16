/****************************************************************
Debugging timer
Copyright 2017 Stable Design <les@sheetcam.com>


This program is free software; you can redistribute it and/or modify
it under the terms of the Mozilla Public License Version 2.0 or later
as published by the Mozilla foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License for more details.

You should have received a copy of the Mozilla Public License
along with this program; if not, you can obtain a copy from mozilla.org
******************************************************************/




#include "timer.h"

#ifdef WIN32
#include "windows.h"
#elif _POSIX_C_SOURCE >+ 199309L
#include <time.h> //for nanosleep
#else
#include <unistd.h> //for usleep
#endif

#include <stdlib.h>
#include <stdio.h>

void SleepMs(const unsigned int time)
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


UTimer::UTimer()
{
#ifdef _USING_WINDOWS
    QueryPerformanceFrequency(&m_frequency);
#endif
    Restart();
}

void UTimer::Restart()
{
#ifdef _USING_WINDOWS
    QueryPerformanceCounter(&m_time);
#else
    clock_gettime(CLOCK_MONOTONIC, &m_time);
#endif
}

uint64_t UTimer::GetElapsed(const bool restart)
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

TestTimer::TestTimer(const string& msg)
{
    m_msg = msg;
    m_lastTime = 0;
}


void TestTimer::Check()
{
    uint64_t diff = GetElapsed(true);
    if(abs((int64_t)diff - (int64_t)m_lastTime) > m_lastTime / 5)
    {
        m_lastTime = diff;
        printf("%s = %f ms\n", m_msg.c_str(), (float)diff / 1000);
    }
}
