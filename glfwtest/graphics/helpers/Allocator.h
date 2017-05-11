#pragma once

#include <vulkan/vulkan.h>
#include <iostream>

#include "helpers/Singleton.h"


class Allocator
{
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

		std::cout << "~Allocator" << std::endl;
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


};
