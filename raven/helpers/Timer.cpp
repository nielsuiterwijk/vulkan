#include "Timer.h"

#include <atomic>

Timer::Timer()
{
	QueryPerformanceCounter( &_FrameDelay );
	QueryPerformanceCounter( &_CurrentTicks );
	QueryPerformanceFrequency( &_TicksPerSecond ); 
}

Timer::~Timer()
{
}

void Timer::Start()
{
	QueryPerformanceCounter( &_CurrentTicks );
}

void Timer::Stop()
{
	QueryPerformanceCounter( &_FrameDelay );
}

float Timer::GetTimeInSeconds()
{
	return (float)( _FrameDelay.QuadPart - _CurrentTicks.QuadPart ) / ( (float)_TicksPerSecond.QuadPart );
}


double HighResTimer::TicksPerSecond = 0.0;

HighResTimer::HighResTimer()
{

}

HighResTimer::~HighResTimer()
{

}

void HighResTimer::Start()
{
	std::atomic_thread_fence(std::memory_order_seq_cst);
	_Start = __rdtsc();
	std::atomic_thread_fence( std::memory_order_seq_cst );
}

void HighResTimer::Stop()
{
	std::atomic_thread_fence( std::memory_order_seq_cst );
	_End = __rdtsc();
	std::atomic_thread_fence( std::memory_order_seq_cst );
}

float HighResTimer::GetTimeInSeconds()
{
	return float(double(_End - _Start) / TicksPerSecond);
}