#include "Timer.h"

Timer::Timer()
{
	QueryPerformanceCounter( &m_frameDelay );
	QueryPerformanceCounter( &m_currentTicks );
	QueryPerformanceFrequency( &m_ticksPerSecond );
}

Timer::~Timer()
{
}

void Timer::Start()
{
	QueryPerformanceCounter( &m_currentTicks );
}

void Timer::Stop()
{
	QueryPerformanceCounter( &m_frameDelay );
	timeInSeconds = GetTimeInSeconds();
}

float Timer::GetTimeInSeconds()
{
	return (float)( m_frameDelay.QuadPart - m_currentTicks.QuadPart ) / ( (float)m_ticksPerSecond.QuadPart );
}
