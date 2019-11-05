#pragma once

#include <queue>
#include <vector>

#include "Task.h"

class SerialScheduler
{
private:

public:
	void Initialize(size_t numberOfThreads)
	{
		
	}

	std::shared_ptr<Task> Schedule(std::function<void()>&& Callback)
	{
		std::shared_ptr<Task> pTask = std::make_shared<Task>(Callback);
		pTask->Run();
		return pTask;
	}
};
