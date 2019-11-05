#pragma once

#include <queue>
#include <vector>

#include "Task.h"

class ThreadPoolOldStyle
{
private:

public:
	ThreadPoolOldStyle(size_t numberOfThreads) :
		stop(false)
	{
		for (size_t i = 0; i < numberOfThreads; ++i)
		{
			workers.emplace_back([this, i] {
				for (;; )
				{
					std::shared_ptr<STask> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);

						//The condition will take the lock and will wait for to be notified and will continue
						//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

						//Once we passed the condition we have the lock, and as soon we leave the scope, the lock will be given up
						if (this->stop && this->tasks.empty())
						{
							return;
						}

						task = this->tasks.front();
						this->tasks.pop();
					}
					//printf("Thread %d executing task.. \n", (int32_t)i);
					task->callback();
					task->done = true;
				}
				});
		}
	}

	~ThreadPoolOldStyle()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}

	template <class F>
	std::shared_ptr<STask> Enqueue(F&& function) noexcept
	{
		std::shared_ptr<STask> res = std::make_shared<STask>();
		res->done = true;

		{
			//Take the lock, and it will be auto cleared at the end of the scope.
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				return res;

			res->done = false;
			res->callback= std::move(function);

			tasks.emplace(res);
		}
		condition.notify_one();

		return res;
	}

private:

	std::vector<std::thread> workers;
	std::queue< std::shared_ptr<STask> > tasks;

	std::mutex queue_mutex; //TODO: At some point a lockless queue would be a nice addition
	std::condition_variable condition;

	bool stop;
};
