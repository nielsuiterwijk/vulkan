#include "RavenApp.h"

#include "io/FileSystem.h"
#include "memory/FreeListAllocator.h"
#include "memory/PoolAllocator.h"
#include "threading/Task.h"
#include "threading/ThreadPool.h"
#include "threading/ThreadPool_PerThreadQueue.h"
#include "threading/ThreadPool_Serial.h"
#include "threading/TaskSystem.h"

#include "helpers/SystemInfo.h"


#include "memory/BlockAllocator.h"

#include <filesystem>
#include <chrono>
#include <thread>

#include "helpers/measure/measure.h"

#include "zstd/zstd.h"


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

	size_t Cutoff = (size_t) (timings.size() * 0.001); //we care about the 99.9% of the cases..?

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
	

	FileSystem::Start();

	//std::cout << "size of size_t: " << sizeof(size_t) << std::endl;

	SystemInfo Info;
	SystemInfo::FillInfoFromSystem( Info );

	HighResTimer::TicksPerSecond = Info._CPUSpeed * 1000000; //Megahertz to Hertz
	   

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
