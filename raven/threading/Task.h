#pragma once

#include <functional>
#include <future>

struct STask
{
	bool done = false;
	std::function<void()> callback = nullptr;
};

enum class ETaskState
{
	InQueue,
	Started,
	Finished
};

class Task
{
public:
	Task( std::function<void()> callback ) :
		Callback( callback )
	{
	}

	void Run()
	{
		if (Mutex.try_lock())
		{
			InternalRun();
			Mutex.unlock();
		}
	}

	//If function returns, its done
	void Wait()
	{
		if (State == ETaskState::Finished)
			return;

		if (Mutex.try_lock())
		{
			if (State == ETaskState::InQueue)
			{
				State = ETaskState::Started;
				Callback();
				State = ETaskState::Finished;
			}

			Mutex.unlock();
		}

		while (State != ETaskState::Finished)
		{
			std::this_thread::yield();
		}
	}

	static void Wait(std::shared_ptr<Task>& pTask)
	{
		pTask->Wait();
		pTask = nullptr;
	}

private:
	void InternalRun()
	{
		if (State == ETaskState::InQueue)
		{
			State = ETaskState::Started;			
			Callback();
			State = ETaskState::Finished;
		}
	}

private:
	std::mutex Mutex;
	ETaskState State = ETaskState::InQueue;
	std::function<void()> Callback;
};
