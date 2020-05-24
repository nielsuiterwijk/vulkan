#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "vma/vk_mem_alloc.h"
#include <vulkan/vulkan.h>


struct SAllocatedBuffer
{
	VkBuffer Buffer = nullptr;
	VmaAllocation Allocation = nullptr;
};

struct SAllocatedImage
{
	VkImage Image = nullptr;
	VmaAllocation Allocation = nullptr;
};

class GPUAllocator
{
public:
	GPUAllocator();
	~GPUAllocator();

	bool AllocateBuffer( VkBufferCreateInfo* pCreateInfo, VmaMemoryUsage UsageFlag, SAllocatedBuffer& Out );

	bool AllocateImage( VkImageCreateInfo* pImageInfo, VmaMemoryUsage UsageFlag, SAllocatedImage& Out );

	int64_t BytesAllocated() const;

	operator VmaAllocator() const { return _Allocator; }

private:
	VmaAllocator _Allocator;
};
