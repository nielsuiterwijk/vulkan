#pragma once

#include <vulkan/vulkan.h>
#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/helpers/InstanceWrapper.h"

class GPUAllocator
{
public:
	GPUAllocator(int32_t size, int32_t alignment);
	~GPUAllocator();

	void Allocate(const VkBuffer& buffer, VkDeviceMemory* memory, VkMemoryPropertyFlags requiredProperties);

	void AllocateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, InstanceWrapper<VkImage>& image, InstanceWrapper<VkDeviceMemory>& imageMemory);

private:
	int32_t FindProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);

private:
	VkPhysicalDeviceMemoryProperties* memoryProperties;

};