#include "Timer.h"

#include <atomic>

void Timer::Start()
{
	_Start = std::chrono::high_resolution_clock::now();
}

void Timer::Stop()
{
	_Stop = std::chrono::high_resolution_clock::now();
}

float Timer::GetTimeInSeconds()
{
	return std::chrono::duration<float>( _Stop - _Start ).count();
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