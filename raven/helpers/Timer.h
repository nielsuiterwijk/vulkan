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
	LARGE_INTEGER _TicksPerSecond;
	LARGE_INTEGER _CurrentTicks;
	LARGE_INTEGER _FrameDelay;
};


class HighResTimer
{
public:
	static double TicksPerSecond;

	HighResTimer();
	~HighResTimer();

	void Start();
	void Stop();

	float GetTimeInSeconds();

private:
	uint64_t _Start;
	uint64_t _End;

};
