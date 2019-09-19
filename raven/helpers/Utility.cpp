#include "Utility.h"

#include <algorithm>
#include <thread>

int32_t Utility::AvailableHardwareThreads()
{
	int32_t available = std::thread::hardware_concurrency() - 1;
	available = std::max( 1, available );

	return available;
}
