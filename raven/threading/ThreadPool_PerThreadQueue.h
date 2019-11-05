#pragma once

#include <queue>
#include <vector>
#include <memory>

#include "Task.h"

class ThreadPoolPerQueue
{
public:
	ThreadPoolPerQueue( size_t numberOfThreads ) :
		stop( false )
	{
		numberOfThreads = std::min(numberOfThreads, threadInfo.size());
		workers.reserve(numberOfThreads);

		for ( size_t i = 0; i < numberOfThreads; ++i )
		{
			workers.emplace_back( [this, i] {
				for ( ;; )
				{
					ThreadInfo& Info = threadInfo[i];
					std::function<void()> task = nullptr;

					{
						std::unique_lock<std::mutex> lock( Info.queue_mutex );

						//The condition will take the lock and will wait for to be notified and will continue
						//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
						Info.condition.wait( lock, [this, i]
							{
								return this->stop || !this->threadInfo[i].tasks.empty(); 
							} );

						//Once we passed the condition we have the lock, and as soon we leave the scope, the lock will be given up
						if ( this->stop && Info.tasks.empty() )
						{
							return;
						}

						task = std::move(Info.tasks.front() );
						Info.tasks.pop();
					}
					//printf("Thread %d executing task.. \n", (int32_t)i);
					task();
				}
			} );
		}
	}

	~ThreadPoolPerQueue()
	{
		stop = true;
		for (size_t i = 0; i < threadInfo.size(); ++i)
		{
			threadInfo[i].condition.notify_one();
		}

		for ( std::thread& worker : workers )
			worker.join();
	}

	template <class F, class... Args>
	auto Enqueue( F&& function, Args&&... args ) noexcept -> std::future<typename std::result_of<F( Args... )>::type>
	{
		using return_type = typename std::result_of<F( Args... )>::type;

		//Here we create a callback object with the arguments, that way we can use it later (or in a different thread.
		auto task = std::make_shared<std::packaged_task<return_type()>>( std::bind( std::forward<F>( function ), std::forward<Args>( args )... ) );

		//We get the return result back from the object, which will be set at some point in the future.
		std::future<return_type> res = task->get_future();

		int32_t CurrentThread = taskQueue.fetch_add(1);
		CurrentThread = CurrentThread % workers.size();
		ThreadInfo& info = threadInfo[CurrentThread];
		//printf("Thread %d enqueued task.. \n", (int32_t)CurrentThread);
		
		{
			//Take the lock, and it will be auto cleared at the end of the scope.
			std::unique_lock<std::mutex> lock( info.queue_mutex );

			// don't allow enqueueing after stopping the pool
			if (stop)
				return std::future<return_type>();

			info.tasks.emplace( [task]() {
				( *task )();
			} );
		}
		info.condition.notify_one();

		return res;
	}

private:

	struct ThreadInfo
	{
		std::queue<std::function<void()>> tasks = {};
		std::mutex queue_mutex = {};
		std::condition_variable condition = {};
	};

	std::array<ThreadInfo, 12> threadInfo;
	std::vector<std::thread> workers;
	
	std::atomic_int taskQueue = 0;

	bool stop;
};
