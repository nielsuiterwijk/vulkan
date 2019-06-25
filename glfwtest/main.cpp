#include "RavenApp.h"

#include "io/FileSystem.h"
#include "memory/FreeListAllocator.h"
#include "memory/PoolAllocator.h"
#include "threading/Task.h"
#include "threading/ThreadPool.h"

#include "standard.h"
#include <filesystem>

int ThreadedCall( int a, int b )
{
	::Sleep( 10 );
	std::cout << a * b << " = a * b" << std::endl;

	return a * b;
}

void AsyncCallback( std::vector<char> fileData )
{
	std::cout << "received result for AsyncCallback: " << fileData.size() << std::endl;
}

int main()
{
#if DEBUG
	_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF );
#endif

	namespace fs = std::filesystem;
	//https://www.codingame.com/playgrounds/5659/c17-filesystem
	fs::path pathToShow( fs::current_path() );
	std::cout << "exists() = " << fs::exists( pathToShow ) << "\n"
			  << "root_name() = " << pathToShow.root_name() << "\n"
			  << "root_path() = " << pathToShow.root_path() << "\n"
			  << "relative_path() = " << pathToShow.relative_path() << "\n"
			  << "parent_path() = " << pathToShow.parent_path() << "\n"
			  << "filename() = " << pathToShow.filename() << "\n"
			  << "stem() = " << pathToShow.stem() << "\n"
			  << "extension() = " << pathToShow.extension() << "\n";

	int i = 0;
	for ( const auto& part : pathToShow )
		std::cout << "path part: " << i++ << " = " << part << "\n";

	std::vector<std::string> filesInDirectory;
	IO::ListFiles( pathToShow.string(), filesInDirectory );

	//SearchTest::Run();

	std::vector<int> test = { 0, 5, 10, 15 };

	auto Filter = [=]( const int& Offer ) -> bool { return Offer == 10; };

	FileSystem::Start();

	//std::cout << "size of size_t: " << sizeof(size_t) << std::endl;

	{
		RavenApp app;

		if ( !app.Initialize() )
		{
			return EXIT_FAILURE;
		}

		try
		{
			app.Run();
		}
		catch ( const std::runtime_error& e )
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
