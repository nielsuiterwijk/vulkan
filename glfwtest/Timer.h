#pragma once

#include <windows.h>

class Timer
{
public:
	Timer();	
	~Timer();

	void Start();
	void Stop();

	float GetTimeInSeconds();

private:
	LARGE_INTEGER	m_ticksPerSecond;
	LARGE_INTEGER	m_currentTicks;
	LARGE_INTEGER	m_frameDelay;
};