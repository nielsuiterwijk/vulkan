#include "FileSystem.h"
#include "threading/ThreadPool.h"

bool FileSystem::stop = false;
bool FileSystem::threadStarted = false;

std::thread FileSystem::fileLoadingThread;
std::queue<FileSystem::AsyncFileLoad> FileSystem::tasks;

Mutex FileSystem::queue_mutex;
std::condition_variable_any FileSystem::condition;

ThreadPool* FileSystem::threadPool = nullptr;

void FileSystem::Start()
{
	ASSERT( !threadStarted );

	std::cout << "[FileSystem] starting thread.." << std::endl;

	threadPool = new ThreadPool( std::min( Utility::AvailableHardwareThreads(), 3 ) );

	fileLoadingThread = std::thread( LoadAsync );
}

void FileSystem::Exit()
{
	std::cout << "[FileSystem] stopping thread.." << std::endl;

	stop = true;

	queue_mutex.lock();
	condition.notify_one();
	queue_mutex.unlock();

	fileLoadingThread.join();

	delete threadPool;
	threadPool = nullptr;
}

std::vector<char> FileSystem::ReadFile( const std::string& filename )
{
	std::vector<char> buffer;
	std::ifstream file( "../assets/" + filename, std::ios::binary | std::ios::ate);

	if ( !file.is_open() )
	{
		std::cout << "[FileSystem] Failed loading " << filename.c_str() << std::endl;
		ASSERT_FAIL( "failed to open file!" );
		return buffer;
	}

	file.seekg(0, file.end);

	size_t fileSize = file.tellg();
	buffer.resize(fileSize);

	file.seekg(0, file.beg);


	file.read( buffer.data(), fileSize );


	file.close();

	std::cout << "[FileSystem] Loaded " << filename.c_str() << " size: " << fileSize << std::endl;

#if DEBUG
	//Sleep(100);
#endif

	return buffer;
}

//TODO: This callback should probably expect some sort of `Result` object which will either contain the data or an error
void FileSystem::LoadFileAsync( const std::string& fileName, std::function<void( std::vector<char> )> callback )
{
	ASSERT( threadStarted );
	ASSERT( fileName.length() != 0 );

	std::cout << "[FileSystem] Queuing loading " << fileName.c_str() << std::endl;

	queue_mutex.lock();
	tasks.push( AsyncFileLoad( fileName, callback ) );
	queue_mutex.unlock();

	condition.notify_one();
}

void FileSystem::LoadAsync()
{
	threadStarted = true;
	stop = false;

	std::cout << "[FileSystem] thread started" << std::endl;

	while ( !stop )
	{
		AsyncFileLoad loadTask;

		{
			std::unique_lock<Mutex> lock( queue_mutex );

			//The condition will take the lock and will wait for to be notified and will continue
			//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
			condition.wait( lock, [] { return stop || !tasks.empty(); } );

			//Once we passed the condition we have the lock, and as soon we leave the scope, the lock will be given up
			if ( stop )
			{
				std::cout << "[FileSystem] thread stopped" << std::endl;
				threadStarted = false;
				return;
			}

			loadTask = tasks.front();
			tasks.pop();
		}

		std::vector<char> fileData = ReadFile( loadTask.fileName );

		threadPool->Enqueue( [=]() {
			std::cout << "[FileSystem] callback -> " << loadTask.fileName << std::endl;
			loadTask.callback( fileData );
		} );
	}
}
