#pragma once

#include <functional>
#include <future>


class Task
{
public:
	Task(std::function<void()> callback) :
		callback(callback)
	{

	}

	void Run()
	{
		callback();
	}

private:
	std::function<void()> callback;
};