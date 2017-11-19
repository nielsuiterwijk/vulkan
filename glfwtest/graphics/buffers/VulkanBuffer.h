#pragma once

#include <vulkan/vulkan.h>

class VulkanBuffer
{
public:
	VulkanBuffer(void* data, size_t size);
	~VulkanBuffer();

	const VkBuffer& GetNative() const
	{
		return nativeBuffer;
	}
	
	VkDeviceMemory* GetDeviceMemory()
	{
		return &nativeMemory;
	}

private:
	VkBuffer nativeBuffer;
	VkDeviceMemory nativeMemory;
};