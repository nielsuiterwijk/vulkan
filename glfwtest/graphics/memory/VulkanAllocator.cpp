#include "VulkanAllocator.h"

#include <iostream>
#include <malloc.h>


void* VulkanAllocator::Allocate(size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	//std::cout << "[Vulkan]Allocated " << size << " bytes. scope: " << scope << std::endl;

	void* address = _aligned_malloc(size, alignment);

	trackers[scope].allocatedMemory.insert(std::make_pair(address, size));
	trackers[scope].totalMemoryAllocated += size;

	trackers[scope].totalAllocations++;

	//std::cout << "[Vulkan] Total allocated " << trackers[scope].totalMemoryAllocated << " bytes. type: " << Vulkan::GetAllocationScopeName(scope) << std::endl;

	return address;
}

void* VulkanAllocator::Reallocate(void* address, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	std::map<void*, size_t>::iterator it = trackers[scope].allocatedMemory.find(address);

	if (it != trackers[scope].allocatedMemory.end())
	{
		trackers[scope].totalMemoryAllocated -= it->second;
		trackers[scope].totalMemoryAllocated += size;

		trackers[scope].totalReallocations++;
	}

	return _aligned_realloc(address, size, alignment);
}

void VulkanAllocator::Free(void* address)
{
	for (size_t i = 0; i < trackers.size(); i++)
	{
		std::map<void*, size_t>::iterator it = trackers[i].allocatedMemory.find(address);

		if (it != trackers[i].allocatedMemory.end())
		{
			trackers[i].totalMemoryAllocated -= it->second;
			trackers[i].allocatedMemory.erase(it);

			//std::cout << "[Vulkan] Total allocated " << trackers[i].totalMemoryAllocated << " bytes. type: " << Vulkan::GetAllocationScopeName(trackers[i].scope) << std::endl;
			break;
		}
	}


	_aligned_free(address);
}

void VulkanAllocator::InternalAllocation(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	std::cout << "[Vulkan]Allocated " << size << " bytes. type: " << allocationType << " scope: " << scope << std::endl;
}

void VulkanAllocator::InternalFree(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	std::cout << "[Vulkan]Freed " << size << " bytes. type: " << allocationType << " scope: " << scope << std::endl;
}
