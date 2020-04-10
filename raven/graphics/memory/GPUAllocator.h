#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "vma/vk_mem_alloc.h"
#include <vulkan/vulkan.h>


struct SAllocatedBuffer
{
	VkBuffer Buffer;
	VmaAllocation Allocation;
};

struct SAllocatedImage
{
	VkImage Image;
	VmaAllocation Allocation;
};

class GPUAllocator
{
public:
	GPUAllocator();
	~GPUAllocator();

	bool AllocateBuffer( VkBufferCreateInfo* pCreateInfo, VmaMemoryUsage UsageFlag, SAllocatedBuffer& Out );

	bool AllocateImage( VkImageCreateInfo* pImageInfo, VmaMemoryUsage UsageFlag, SAllocatedImage& Out );

	VmaAllocator Get() const { return _Allocator; }

private:
	VmaAllocator _Allocator;
};
