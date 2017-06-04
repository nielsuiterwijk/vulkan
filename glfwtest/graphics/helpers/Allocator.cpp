#include "Allocator.h"

#include <iostream>
#include <malloc.h>


void* Allocator::Allocate(size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	//std::cout << "[Vulkan]Allocated " << size << " bytes. scope: " << scope << std::endl;

	void* allocated = _aligned_malloc(size, alignment);

	trackers[scope].allocatedMemory.insert(std::make_pair(allocated, size));
	trackers[scope].totalMemoryAllocated += size;

	//std::cout << "[Vulkan] Total allocated " << trackers[scope].totalMemoryAllocated << " bytes. type: " << Vulkan::GetAllocationScopeName(scope) << std::endl;

	return allocated;
}

void* Allocator::Reallocate(void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	return _aligned_realloc(original, size, alignment);
}

void Allocator::Free(void* data)
{
	for (size_t i = 0; i < trackers.size(); i++)
	{
		std::map<void*, size_t>::iterator it = trackers[i].allocatedMemory.find(data);

		if (it != trackers[i].allocatedMemory.end())
		{
			trackers[i].totalMemoryAllocated -= it->second;
			trackers[i].allocatedMemory.erase(it);

			//std::cout << "[Vulkan] Total allocated " << trackers[i].totalMemoryAllocated << " bytes. type: " << Vulkan::GetAllocationScopeName(trackers[i].scope) << std::endl;
			break;
		}
	}


	_aligned_free(data);
}

void Allocator::InternalAllocation(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	std::cout << "[Vulkan]Allocated " << size << " bytes. type: " << allocationType << " scope: " << scope << std::endl;
}

void Allocator::InternalFree(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	std::cout << "[Vulkan]Freed " << size << " bytes. type: " << allocationType << " scope: " << scope << std::endl;
}
