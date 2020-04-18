#pragma once

#include <windows.h>

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	void Start();
	void Stop();

	float GetTimeInSeconds();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _Start = {};
	std::chrono::time_point<std::chrono::high_resolution_clock> _Stop = {};
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
