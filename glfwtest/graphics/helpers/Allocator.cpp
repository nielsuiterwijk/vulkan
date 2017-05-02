#include "Allocator.h"

#include <iostream>
#include <malloc.h>


void* Allocator::Allocate(size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	std::cout << "[Vulkan]Allocated " << size << " bytes. scope: " << scope << std::endl;

	return _aligned_malloc(size, alignment);
}

void* Allocator::Reallocate(void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	return _aligned_realloc(original, size, alignment);
}

void Allocator::Free(void* data)
{
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
