#include "RavenApp.h"

#include "threading/Task.h"
#include "threading/ThreadPool.h"
#include "io/FileSystem.h"
#include "memory/PoolAllocator.h"
#include "memory/FreeListAllocator.h"

#include "standard.h"
#include "helpers/Integer.h"

#undef min
#include "misc/cache_binary.h"


int ThreadedCall(int a, int b)
{
	::Sleep(10);
	std::cout << a * b << " = a * b" << std::endl;

	return a * b;
}

void AsyncCallback(std::vector<char> fileData)
{
	std::cout << "received result for AsyncCallback: " << fileData.size() << std::endl;
}



int main()
{
#if DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	//SearchTest::Run();

	std::vector<int> test = { 0, 5, 10, 15 };

	auto Filter = [=](const int& Offer) -> bool { return Offer == 10; };
	
	
			
	FileSystem::Start();

	//std::cout << "size of size_t: " << sizeof(size_t) << std::endl;


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

	FileSystem::Exit();

	return EXIT_SUCCESS;
}