#pragma once

#include <vulkan/vulkan.h>
#include "graphics/buffers/VulkanBuffer.h"

class GPUAllocator
{
public:
	GPUAllocator(int32_t size, int32_t alignment);
	~GPUAllocator();

	void Allocate(VulkanBuffer& buffer);

private:
	int32_t FindProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);

private:
	VkPhysicalDeviceMemoryProperties* memoryProperties;

};