#include "RavenApp.h"

#include "io/FileSystem.h"
#include "memory/FreeListAllocator.h"
#include "memory/PoolAllocator.h"
#include "threading/Task.h"
#include "threading/ThreadPool.h"
#include "threading/ThreadPool_PerThreadQueue.h"
#include "threading/ThreadPool_OldStyle.h"
#include "threading/ThreadPool_Serial.h"
#include "threading/TaskSystem.h"

#include "standard.h"
#include <filesystem>
#include <chrono>

#include "helpers/measure/measure.h"

#include "random_generator.h"

#include "zstd/zstd.h"

#include "misc/avx.h"

namespace NPdxUtils
{
	template <class T>
	void PdxMerge(std::vector<T> a, std::vector<T> b)
	{
		std::vector<T> d;
		d.resize(a.size() + b.size());
		std::merge(a.begin(), a.end(), b.begin(), b.end(), d.begin());
	}
}

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

struct STimePoint
{
	double count;
	char padding[56];
};

void PrintStats(const char* label, std::vector<STimePoint>& timings)
{
	std::sort(timings.begin(), timings.end(), [](const STimePoint& Left, const STimePoint& Right) { return Left.count > Right.count; });

	double fastest = std::numeric_limits<double>::max();
	double slowest = std::numeric_limits<double>::min();

	size_t Cutoff = timings.size() * 0.001; //we care about the 99.9% of the cases..?

	std::cout << std::fixed << std::setprecision(9) << label;
	fastest = timings.back().count;
	slowest = timings.front().count;
	for (size_t i = Cutoff; i < timings.size(); ++i)
	{
		//fastest = std::min(fastest, timings[i].count);
		slowest = std::max(slowest, timings[i].count);
		break;
		//std::cout << timings[i] << ",";
	}

	double sum = 0.0;
	for (size_t i = Cutoff; i < timings.size(); ++i) {
		sum += timings[i].count;
	}
	double avg = sum / static_cast<double>(timings.size() - Cutoff);

	sum = 0.0;
	for (size_t i = Cutoff; i < timings.size(); ++i)
	{
		double timing = pow(timings[i].count - avg, 2);
		sum += timing;
	}
	double var = sum / (timings.size() - Cutoff);
	double sdv = sqrt(var);

	std::cout << "average " << avg << "s, fastest " << fastest << "s, slowest " << slowest << "s, stddev " << sdv << "s";
	std::cout << std::endl;
}

static_assert(sizeof(STimePoint) == 64, "unexpected size");

int main()
{
#if DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif
	/*
	AVXTest::Test();
	AVXTest::TestMatrix();


	std::cout << std::fixed << std::setprecision(9);
	std::cout << "high_precision " << (double)std::chrono::high_resolution_clock::period::num / std::chrono::high_resolution_clock::period::den << "s" << std::endl;
	std::cout << "steady         " << (double)std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den << "s" << std::endl;

	//{
	//	const char* my_data = "aaaaaaabbbbbbbbbccccccccddddddddeeeeeeeefffffffffggggggggg";
	//	size_t size_of_data = strlen(my_data);
	//
	//	size_t estimated_size_of_compressed_data = ZSTD_compressBound(size_of_data);
	//	void* compressed_data = malloc(estimated_size_of_compressed_data);
	//
	//	size_t actual_size_of_compressed_data = ZSTD_compress(compressed_data, estimated_size_of_compressed_data, my_data, size_of_data, ZSTD_CLEVEL_DEFAULT);
	//
	//	size_t estimated_size_of_decompressed_data = ZSTD_getFrameContentSize(compressed_data, estimated_size_of_compressed_data);
	//	void* decompressed_data = malloc(estimated_size_of_decompressed_data);
	//
	//	size_t actual_size_of_decompressed_data = ZSTD_decompress(decompressed_data, estimated_size_of_decompressed_data, compressed_data, actual_size_of_compressed_data);
	//
	//	FILE* fp = fopen("/tmp/mydata.zst", "wb");
	//	fwrite(compressed_data, actual_size_of_compressed_data, 1, fp);
	//	fclose(fp);
	//	delete compressed_data;
	//}

	constexpr int NumTests = 100000;
	using namespace std::chrono_literals;

	auto SlowCall = []()
	{
		std::this_thread::sleep_for(5s);
	};

	//Task Task(SlowCall);

	TaskSystem<SerialScheduler> SerialTaskSystem(2);

	std::shared_ptr<Task> pTask = SerialTaskSystem.Enqueue(SlowCall);
	Task::Wait(pTask);

	//NEXT: Make a generic interface and allow 'different' schedulers?
	{
		ThreadPool DefaultPool(Utility::AvailableHardwareThreads());

		std::vector<STimePoint> Timings;
		Timings.resize(NumTests);

		std::vector<STimePoint> Timings2;
		Timings2.resize(NumTests);

		std::vector<std::future<void>> Results;
		Results.resize(NumTests);

		for (int i = 0; i < NumTests; i++)
		{
			const std::chrono::high_resolution_clock::time_point Start = std::chrono::high_resolution_clock::now();

			const auto Callback = [&Timings, Start, i]()
			{
				std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
				double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
				Timings[i].count = deltaCount;
			};

			Results[i] = DefaultPool.Enqueue(Callback);


			const std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
			const double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
			Timings2[i].count = deltaCount;
		}

		for (int i = 0; i < NumTests; i++)
		{
			Results[i].wait();
		}
		
		PrintStats("SPMC Callback called - ", Timings);
		PrintStats("SPMC enqueue -         ", Timings2);
	}
	std::cout << std::endl;
	//Sleep(5000);
	{
		ThreadPoolPerQueue DefaultPool(Utility::AvailableHardwareThreads());

		std::vector<STimePoint> Timings;
		Timings.resize(NumTests);

		std::vector<STimePoint> Timings2;
		Timings2.resize(NumTests);

		std::vector<std::future<void>> Results;
		Results.resize(NumTests);

		for (int i = 0; i < NumTests; i++)
		{
			const std::chrono::high_resolution_clock::time_point Start = std::chrono::high_resolution_clock::now();

			auto Callback = [&Timings, Start, i]()
			{
				std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
				double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
				Timings[i].count = deltaCount;
			};

			Results[i] = DefaultPool.Enqueue(Callback);

			const std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
			const double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
			Timings2[i].count = deltaCount;
		}

		for (int i = 0; i < NumTests; i++)
		{
			Results[i].wait();
		}
		
		PrintStats("SPSC Callback called - ", Timings);
		PrintStats("SPSC enqueue -         ", Timings2);
	}
	std::cout << std::endl;
	//Sleep(5000);
	{
		ThreadPoolOldStyle DefaultPool(Utility::AvailableHardwareThreads());

		std::vector<STimePoint> Timings;
		Timings.resize(NumTests);

		std::vector<STimePoint> Timings2;
		Timings2.resize(NumTests);

		std::vector< std::shared_ptr<STask> > Results;
		Results.resize(NumTests);

		for (int i = 0; i < NumTests; i++)
		{
			const std::chrono::high_resolution_clock::time_point Start = std::chrono::high_resolution_clock::now();

			const auto Callback = [&Timings, Start, i]()
			{
				std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
				double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
				Timings[i].count = deltaCount;
			};

			Results[i] = DefaultPool.Enqueue(Callback);

			const std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
			const double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
			Timings2[i].count = deltaCount;
		}

		for (int i = 0; i < NumTests; i++)
		{
			while (Results[i]->done == false) { Sleep(0); }
		}
		
		PrintStats("C Callback called -    ", Timings);
		PrintStats("C enqueue -            ", Timings2);
	}
	std::cout << std::endl;
	{
		TaskSystem<SerialScheduler> SerialTaskSystem(4);

		std::vector<STimePoint> Timings;
		Timings.resize(NumTests);

		std::vector<STimePoint> Timings2;
		Timings2.resize(NumTests);

		std::vector< std::shared_ptr<Task> > Results;
		Results.resize(NumTests);

		for (int i = 0; i < NumTests; i++)
		{
			const std::chrono::high_resolution_clock::time_point Start = std::chrono::high_resolution_clock::now();

			const auto Callback = [&Timings, Start, i]()
			{
				std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
				double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
				Timings[i].count = deltaCount;
			};

			Results[i] = SerialTaskSystem.Enqueue(Callback);

			const std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
			const double deltaCount = std::chrono::duration_cast<std::chrono::duration<double>>(End - Start).count();
			Timings2[i].count = deltaCount;
		}

		for (int i = 0; i < NumTests; i++)
		{
			Task::Wait(Results[i]);
		}
		
		PrintStats("Baseline direct call - ", Timings);
		PrintStats("Total enqueue -        ", Timings2);
	}


	std::mutex mutex;

	auto a123 = Measures::MeasureShort([&mutex]()
		{
			std::scoped_lock<std::mutex> ScopedLock(mutex);
			return 42;
		});


	auto b123 = Measures::MeasureShort([&mutex]()
		{
			return 42;
		});

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

	fast_srand(42);
	srand_sse(42);

	unsigned int* buffer = new unsigned int[4];


	std::vector<int32_t> largeArray;
	std::vector<int32_t> smallArray;
	std::vector<int32_t> verySmallArray;

	for (int i = 0; i < 1000000; i++)
	{
		largeArray.emplace_back(fastrand());
	}

	for (int i = 0; i < 10000; i++)
	{
		smallArray.emplace_back(fastrand());
	}

	for (int i = 0; i < 100; i++)
	{
		verySmallArray.emplace_back(fastrand());
	}

	//Measures::Print(Measures::MeasureShort([]() { return 8 * 8; }), "test");
	//
	//Measures::Print(Measures::MeasureLong([largeArray]() {
	//	std::vector<int32_t> largeArrayCopy = largeArray;
	//	std::sort(std::begin(largeArrayCopy), std::end(largeArrayCopy), std::less<int32_t>());
	//	return largeArrayCopy[3] == largeArray[3];
	//	}), "large array");
	//
	//Measures::Print(Measures::MeasureLong([smallArray]() {
	//	std::vector<int32_t> copy = smallArray;
	//	std::sort(std::begin(copy), std::end(copy), std::less<int32_t>());
	//	return copy[3] == smallArray[3];
	//	}), "small array");
	//
	//Measures::Print(Measures::MeasureLong([verySmallArray]() {
	//	std::vector<int32_t> copy = verySmallArray;
	//	std::sort(std::begin(copy), std::end(copy), std::less<int32_t>());
	//	return copy[3] == verySmallArray[3];
	//	}), "v small array");
	//
	//
	//
	//Measures::Print(Measures::MeasureShort([]() {
	//	return fastrand() + fastrand() + fastrand() + fastrand();
	//	}), "sse");
	//
	//Measures::Print(Measures::MeasureShort([buffer]() {
	//	rand_sse(buffer);
	//	return buffer[0] + buffer[1] + buffer[2] + buffer[3];
	//	}), "fast_c");
		

	
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

	auto Filter = [=]( const int& Offer ) -> bool { return Offer == 10; };*/

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
