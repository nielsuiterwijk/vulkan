#include "FileSystem.h"

bool FileSystem::stop = false;
bool FileSystem::threadStarted = false;

std::thread FileSystem::fileLoadingThread;
std::queue<FileSystem::AsyncFileLoad> FileSystem::tasks;

std::mutex FileSystem::queue_mutex; 
std::condition_variable FileSystem::condition;


void FileSystem::Start()
{
	assert(!threadStarted);

	std::cout << "[FileSystem] starting thread.." << std::endl;
	fileLoadingThread = std::thread(LoadAsync);
}

void FileSystem::Exit()
{
	std::cout << "[FileSystem] stopping thread.." << std::endl;
	stop = true;

	queue_mutex.lock();
	condition.notify_one();
	queue_mutex.unlock();

	fileLoadingThread.join();
}

std::vector<char> FileSystem::ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "[FileSystem] Failed loading " << filename.c_str() << std::endl;
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	std::cout << "[FileSystem] Loaded " << filename.c_str()  << " size: " << fileSize << std::endl;

#if DEBUG
	//::_sleep(1000);
#endif

	return buffer;
}

void FileSystem::LoadFileAsync(const std::string& fileName, std::function<void(std::vector<char>)> callback)
{
	assert(threadStarted);
	assert(fileName.length() != 0);

	std::cout << "[FileSystem] Queuing loading " << fileName.c_str() << std::endl;

	queue_mutex.lock();
	tasks.push(AsyncFileLoad(fileName, callback));
	queue_mutex.unlock();

	condition.notify_one();
}

void FileSystem::LoadAsync()
{
	threadStarted = true;
	stop = false;

	std::cout << "[FileSystem] thread started" << std::endl;

	while (!stop)
	{
		AsyncFileLoad loadTask;

		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			//The condition will take the lock and will wait for to be notified and will continue
			//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
			condition.wait(lock, [] { return stop || !tasks.empty(); });

			//Once we passed the condition we have the lock, and as soon we leave the scope, the lock will be given up
			if (stop)
			{
				std::cout << "[FileSystem] thread stopped" << std::endl;
				threadStarted = false;
				return;
			}

			loadTask = tasks.front();
			tasks.pop();
		}

		loadTask.callback(ReadFile(loadTask.fileName));
	}
}