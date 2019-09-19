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

	int8_t a = 0;
	int16_t b = 0;
	int32_t c = 0;
	int64_t d = 0;

	std::vector<int32_t> test1 = { 0, 5, 10, 15 };
	std::vector<int32_t> test2 = { 0, 5, 10, 15 };

	typedef std::pair<std::string, int32_t> KeyValue;

	std::vector<KeyValue> test3 = { { "doortje", 20 }, { "niels", 10 }, { "dikkie", 11 } };

	std::sort( std::begin( test3 ), std::end( test3 ), std::less<KeyValue>() );
	std::sort( std::begin( test3 ), std::end( test3 ), std::greater<KeyValue>() );

	auto SortOnValueGreater = []( const KeyValue& left, const KeyValue& right ) {
		return left.second > right.second;
	};

	auto SortOnValueLess = []( const KeyValue& left, const KeyValue& right ) {
		return left.second < right.second;
	};

	std::sort( std::begin( test3 ), std::end( test3 ), SortOnValueGreater );
	std::sort( std::begin( test3 ), std::end( test3 ), SortOnValueLess );

	bool isSame = test1 == test2;

	std::sort( std::begin( test1 ), std::end( test1 ), std::greater<int32_t>() );

	auto greaterLamda = []( int32_t left, int32_t right ) {
		return left > right;
	};

	std::sort( std::begin( test1 ), std::end( test1 ), std::less<int32_t>() );

	bool isSame2 = test1 == test2;

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
