#pragma once

#include "standard.h"

#include <fstream>
#include <queue>


class FileSystem
{
private:
	struct AsyncFileLoad
	{
	public:
		AsyncFileLoad() :
			fileName("default"),
			callback(nullptr)
		{

		}

		AsyncFileLoad(const std::string& fileName, std::function<void(std::vector<char>)> callback) :
			fileName(fileName),
			callback(callback)
		{

		}

		AsyncFileLoad& operator=(const AsyncFileLoad& other) // copy assignment
		{
			// self-assignment check expected
			if (this != &other) 
			{ 
				fileName = other.fileName;
				callback = other.callback;
			}
			return *this;
		}

		std::string fileName;
		std::function<void(std::vector<char>)> callback;
	};
public:

	static void Start();
	static void Exit();

	static std::vector<char> ReadFile(const std::string& filename);

	static void LoadFileAsync(const std::string& fileName, std::function<void(std::vector<char>)> callback);

private:
	static void LoadAsync();

private:
	static std::thread fileLoadingThread;
	static std::queue<AsyncFileLoad> tasks;

	static std::mutex queue_mutex; //TODO: At some point a lockless queue would be a nice addition
	static std::condition_variable condition;

	static bool threadStarted;
	static bool stop;
};