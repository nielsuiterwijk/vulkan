#pragma once

#include "Task.h"

template <class T>
class TaskSystem
{
public:
	TaskSystem( size_t numberOfThreads )
	{
		pScheduler = new T();
		pScheduler->Initialize(numberOfThreads);
	}

	inline std::shared_ptr<Task> Enqueue(std::function<void()> Callback)
	{
		return pScheduler->Schedule(std::move(Callback));
	}

private:
	T* pScheduler;
};
