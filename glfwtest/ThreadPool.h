#pragma once


class ThreadPool
{
public:
	ThreadPool(size_t numberOfThreads)
	{

	}

	template<class F, class... Args>
	auto Enqueue(F&& f, Args&&... args) ->std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<return_type> res = task->get_future();

		//TODO: put into a queue
		{
			Task taskObject([task]()
			{
				(*task)();
			});

			taskObject.Run();
		}

		return res;
	}
};