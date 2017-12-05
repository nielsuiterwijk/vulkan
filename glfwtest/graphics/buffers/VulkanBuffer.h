#pragma once

#include <vulkan/vulkan.h>

struct BufferType
{
	enum Enum
	{
		Static,
		Dynamic
	};
};

class VulkanBuffer
{
public:
	VulkanBuffer(VkBufferUsageFlags flags, BufferType::Enum bufferType, void* data, size_t size);
	~VulkanBuffer();

	const VkBuffer& GetNative() const
	{
		return deviceBuffer;
	}
	
	VkDeviceMemory* GetDeviceMemory()
	{
		return &nativeMemory;
	}

	void CopyStagingToDevice();

private:
	void SetupStagingBuffer(void* bufferData);
	void SetupLocalStaticBuffer(VkBufferUsageFlags flags);

	void SetupLocalDynamicBuffer(void* bufferData, VkBufferUsageFlags flags);

private:
	VkBuffer stagingBuffer;
	VkBuffer deviceBuffer;

	VkDeviceMemory stagingMemory;
	VkDeviceMemory nativeMemory;

	size_t size;
};