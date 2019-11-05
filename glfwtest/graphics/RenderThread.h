#pragma once

#include "helpers/Timer.h"

class CRenderThread
{
public:
	CRenderThread(std::condition_variable& UpdateRunCondition) :
		_UpdateRunCondition(UpdateRunCondition)
	{

	}


	void Start();

	void Stop();



private:
	void DoFrame();

	std::condition_variable _RenderRunCondition;
	uint64_t _RenderFrame;
	bool _ShouldRun = true;

	std::mutex _NotificationMutex;
	std::condition_variable& _UpdateRunCondition;

	float _AccumelatedTime;
	Timer _TotalTimer;
	Timer _AcquireTimer;
	Timer _DrawCallTimer;
	Timer _RenderQueuTimer;
	Timer _PresentTimer;
};