#include "Integer.h"

#include "RavenApp.h"
#include "Task.h"
#include "ThreadPool.h"

#include <windows.h>
#include <iostream>
#include <stdexcept>


int ThreadedCall(int a, int b)
{
	::Sleep(100);
	std::cout << a * b << " = a * b" << std::endl;

	return a * b;
}

int main()
{
	Integer::RunSanityCheck();

	ThreadPool p(8);
	std::future<int> result = p.Enqueue(ThreadedCall, 3, 7);

	while (result.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
	{
		std::cout << "Waiting for result." << std::endl;
	}

	std::cout << "received result for ThreadedCalled: " << result.get() << std::endl;

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
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}