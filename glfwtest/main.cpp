#include "Integer.h"

#include "RavenApp.h"
#include "Task.h"
#include "ThreadPool.h"

#include <iostream>
#include <stdexcept>


int ThreadedCall(int a, int b)
{
	std::cout << a * b << " = a * b" << std::endl;

	return a * b;
}

int main()
{
	Integer::RunSanityCheck();

	ThreadPool p(8);
	auto result = p.Enqueue(ThreadedCall, 3, 7);

	if (result.valid())
	{
		std::cout << "received result for ThreadedCalled: " << result.get() << std::endl;
	}

	RavenApp app;

	app.Initialize();

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