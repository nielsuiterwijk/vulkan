#pragma once

#include <vulkan/vulkan.h>
#include "VulkanHelpers.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <map>

#include "helpers/Singleton.h"


class Allocator
{
private:
	struct AllocationTracker
	{
	public:
		AllocationTracker(VkSystemAllocationScope scope) :
			scope(scope),
			totalMemoryAllocated(0),
			previousTotalMemoryAllocated(0)
		{
		}

		std::map<void*, size_t> allocatedMemory;

		int64_t totalMemoryAllocated;
		int64_t previousTotalMemoryAllocated;

		VkSystemAllocationScope scope;
	};
public:
	Allocator() :
		vulkanAllocator()
	{
		vulkanAllocator.pUserData = (void*)this;
		vulkanAllocator.pfnAllocation = &Allocator::Allocate;
		vulkanAllocator.pfnReallocation = &Allocator::Reallocate;
		vulkanAllocator.pfnFree = &Allocator::Free;
		vulkanAllocator.pfnInternalAllocation = &Allocator::InternalAllocationNotification;
		vulkanAllocator.pfnInternalFree = &Allocator::InternalFreeNotification;

		/*   = 0,
		 = 1,
		 = 2,
		 = 3,
		 = 4,*/

		trackers.push_back(AllocationTracker(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
		trackers.push_back(AllocationTracker(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
		trackers.push_back(AllocationTracker(VK_SYSTEM_ALLOCATION_SCOPE_CACHE));
		trackers.push_back(AllocationTracker(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
		trackers.push_back(AllocationTracker(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));

		std::cout << "Allocator" << std::endl;
	}

	~Allocator()
	{
		vulkanAllocator.pUserData = nullptr;
		vulkanAllocator.pfnAllocation = nullptr;
		vulkanAllocator.pfnReallocation = nullptr;
		vulkanAllocator.pfnFree = nullptr;
		vulkanAllocator.pfnInternalAllocation = nullptr;
		vulkanAllocator.pfnInternalFree = nullptr;

		trackers.clear();

		std::cout << "~Allocator" << std::endl;
	}

	void PrintStats()
	{
		for (size_t i = 0; i < trackers.size(); i++)
		{
			int64_t memoryAllocated = trackers[i].totalMemoryAllocated;
			int64_t previousMemoryAllocated = trackers[i].previousTotalMemoryAllocated;
			int32_t delta = memoryAllocated - previousMemoryAllocated;

			//Feels like a waste, but nice formatting is nice.
			if (delta > 0)
			{
				std::cout << "[Vulkan] " << Vulkan::GetAllocationScopeName(trackers[i].scope) << " allocated " << memoryAllocated << " bytes.  delta: +" << delta << " bytes." << std::endl;
			}
			else
			{
				std::cout << "[Vulkan] " << Vulkan::GetAllocationScopeName(trackers[i].scope) << " allocated " << memoryAllocated << " bytes.  delta: " << delta << " bytes." << std::endl;
			}

			trackers[i].previousTotalMemoryAllocated = trackers[i].totalMemoryAllocated;
		}
	}

	VkAllocationCallbacks* Get()
	{
		return &vulkanAllocator;
	}

private:
	static void* VKAPI_CALL Allocate(void* userData, size_t size, size_t alignment, VkSystemAllocationScope scope)
	{
		return static_cast<Allocator*>(userData)->Allocate(size, alignment, scope);
	}

	static void* VKAPI_CALL Reallocate(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
	{
		return static_cast<Allocator*>(userData)->Reallocate(original, size, alignment, scope);
	}

	static void VKAPI_CALL Free(void* userData, void* data)
	{
		static_cast<Allocator*>(userData)->Free(data);
	}

	static void VKAPI_CALL InternalAllocationNotification(void* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
	{
		static_cast<Allocator*>(userData)->InternalAllocation(size, allocationType, scope);
	}

	static void VKAPI_CALL InternalFreeNotification(void* userData, size_t size, VkInternalAllocationType  allocationType, VkSystemAllocationScope scope)
	{
		static_cast<Allocator*>(userData)->InternalFree(size, allocationType, scope);
	}

private:
	void* Allocate(size_t size, size_t alignment, VkSystemAllocationScope scope);

	void* Reallocate(void* original, size_t size, size_t alignment, VkSystemAllocationScope scope);

	void Free(void* data);


	void InternalAllocation(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope);
	void InternalFree(size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope);

private:
	VkAllocationCallbacks vulkanAllocator;

	std::vector<AllocationTracker> trackers;


};
