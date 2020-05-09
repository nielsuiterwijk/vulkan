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

#include "ecs/ComponentInterface.h"
#include "ecs/World.h"
#include "ecs/Entity.h"
#include "ecs/SystemInterface.h"

struct Position
{
	float X;
	float Y;
};

struct Position2
{
	float X;
	float Y;
};

struct ComplexComponent
{
	ComplexComponent()
	{

	}

	ComplexComponent( ComplexComponent& Other )
	{
		*this = Other;
	}

	ComplexComponent& operator=( ComplexComponent& other )
	{
		std::cout << "copy assignment of ComplexComponent\n";
		return *this;
	}

	std::string _FileName;
	int32_t _LineNumber;

	void* _pBuffer;
	int32_t _Size;
};

class BounceSystem : public Ecs::SystemInterface
{
	void Tick( Ecs::World& World, float Delta ) final
	{
		//Iterator<Position> Start = World.Access<Position>();
		//
		//while ( Start )
		//{
		//	Position& Pos = *Start++;
		//}
	}
};

void EcsExample()
{
	Ecs::World World;

	Ecs::Entity Entity = World.Create();
	World.Assign<Position>( Entity );
	const Position* pPos = World.raw<Position>();


	Ecs::Entity Entity2 = World.Create();
	World.Assign<Position>( Entity2, Position { 37, 13 } );
	World.Assign<Position2>( Entity2, Position2 { 13, 37 } );

	World.Assign<ComplexComponent>( Entity2 );

	auto SingleView = World.View<Position>(); //Returns all entities that have a Position component
	auto MultipleView = World.View<Position, Position2>(); //Returns all entities that have a Position AND a Position2 component

	auto MultipleView2 = World.View<Position, ComplexComponent>(); //Returns all entities that have a Position AND a Position2 component

	float x1 = 0;
	for ( const Ecs::Entity Entity : SingleView )
	{
		Position& Pos = SingleView.Get<Position>( Entity );
		x1 += Pos.X;

		//Compile time knows if component is contained in view or not
		//Position2& Pos2 = SingleView.Get<Position2>( Entity );
		//x1 += Pos2.X;
	}

	for ( const Ecs::Entity Entity : MultipleView )
	{
		Position& Pos = MultipleView.Get<Position>( Entity );
		x1 += Pos.X;
		Position2& Pos2 = MultipleView.Get<Position2>( Entity );
		x1 += Pos2.X;

		std::tuple<Position&, Position2&> Both = MultipleView.Get<Position, Position2>( Entity );
		Position& Pos1 = std::get<Position&>( Both );
		Pos1.X -= 1;
		x1 += Pos1.X;

		Position& Pos3 = MultipleView.Get<Position>( Entity );
		x1 += Pos3.X;

		Pos.X -= 1;

		std::tuple<Position, Position2> Both2 = MultipleView.Get<Position, Position2>( Entity );
		Position& Pos12 = std::get<Position>( Both2 );
		Pos12.X -= 1;
		x1 += Pos12.X;
	}

	for ( const Ecs::Entity Entity : MultipleView2 )
	{
		Position& Pos = MultipleView.Get<Position>( Entity );
		x1 += Pos.X;
		ComplexComponent& Pos2 = MultipleView2.Get<ComplexComponent>( Entity );

		std::tuple<Position&, ComplexComponent&> Both = MultipleView2.Get<Position, ComplexComponent>( Entity );
		Position& Pos1 = std::get<Position&>( Both );
		Pos1.X -= 1;
		x1 += Pos1.X;

		std::tuple<Position, ComplexComponent> Both2 = MultipleView2.Get<Position, ComplexComponent>( Entity );
		Position& Pos12 = std::get<Position>( Both2 );
		Pos12.X -= 1;
		x1 += Pos12.X;
	}

	BounceSystem Bounce;
}

int main()
{
#if DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	EcsExample();

	/*Component: 0 with id: -551705722
Component: 0 with id: 928072754*/

	

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
