#include "helpers/Integer.h"

#include "RavenApp.h"
#include "threading/Task.h"
#include "threading/ThreadPool.h"

#include <windows.h>
#include <iostream>
#include <stdexcept>



int ThreadedCall(int a, int b)
{
	::Sleep(10);
	std::cout << a * b << " = a * b" << std::endl;

	return a * b;
}

int main()
{
	Integer::RunSanityCheck();

	std::cout << "size of size_t: " << sizeof(size_t) << std::endl;

	/*ThreadPool threadPool(8);
	std::future<int> result = threadPool.Enqueue(ThreadedCall, 3, 7);

	while (result.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
	{
		std::cout << "Waiting for result." << std::endl;
	}

	std::cout << "received result for ThreadedCalled: " << result.get() << std::endl;*/

	{
		RavenApp app;

		if (!app.Initialize())
		{
			return EXIT_FAILURE;
		}

		try
		{
			app.Run();
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
			int input = 0;
			std::cin >> input;

			return EXIT_FAILURE;
		}
	}

	Sleep(3000);

	return EXIT_SUCCESS;
}