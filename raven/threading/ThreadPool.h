#pragma once

#include "Mutex.h"
#include "Task.h"

#include <queue>
#include <vector>

class ThreadPool
{
public:
	ThreadPool( size_t numberOfThreads )
		: stop( false )
	{
		for ( size_t i = 0; i < numberOfThreads; ++i )
		{
			workers.emplace_back( [ this, i ] {
				for ( ;; )
				{
					std::function<void()> task = nullptr;

					{
						std::unique_lock<Mutex> lock( this->queue_mutex );

						//The condition will take the lock and will wait for to be notified and will continue
						//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
						this->condition.wait( lock, [ this ] { return this->stop || !this->tasks.empty(); } );

						//Once we passed the condition we have the lock, and as soon we leave the scope, the lock will be given up
						if ( this->stop && this->tasks.empty() )
						{
							return;
						}

						task = std::move( this->tasks.front() );
						this->tasks.pop();
					}
					//printf("Thread %d executing task.. \n", (int32_t)i);
					task();
				}
			} );
		}
	}

	~ThreadPool()
	{
		{
			std::unique_lock<Mutex> lock( queue_mutex );
			stop = true;
		}
		condition.notify_all();
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

		{
			//Take the lock, and it will be auto cleared at the end of the scope.
			std::unique_lock<Mutex> lock( queue_mutex );

			// don't allow enqueueing after stopping the pool
			if ( stop )
				return std::future<return_type>();

			tasks.emplace( [ task ]() { ( *task )(); } );
		}
		condition.notify_one();

		return res;
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	Mutex queue_mutex; //TODO: At some point a lockless queue would be a nice addition
	std::condition_variable_any condition;

	bool stop;
};
