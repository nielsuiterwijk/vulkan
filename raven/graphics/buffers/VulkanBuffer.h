#pragma once

#include <vulkan/vulkan.h>

struct BufferType
{
	enum Enum
	{
		Static, //Upload & forget
		Dynamic, //Buffers that reguraly get updated from the gpu, keeps a CPU shadow copy
		Staging //Used for uploading, for example, pixels to the gpu but then transition those into an image buffer.
	};
};

class VulkanBuffer
{
public:
	VulkanBuffer( VkBufferUsageFlags flags, BufferType::Enum bufferType, void* data, VkDeviceSize size );
	~VulkanBuffer();

	const VkBuffer& GetNative() const
	{
		return deviceBuffer;
	}

	VkDeviceMemory* GetDeviceMemory()
	{
		return &nativeMemory;
	}

	VkDeviceSize GetSize() const { return size; }

	void CopyStagingToDevice();
	void CopyStagingToImage( VkImage image, uint32_t width, uint32_t height );

	void Map( void* bufferData, VkDeviceSize sizeToMap = -1 ) const;

private:
	void SetupStagingBuffer();
	void SetupLocalStaticBuffer( VkBufferUsageFlags flags );

	void SetupLocalDynamicBuffer( void* bufferData, VkBufferUsageFlags flags );

	void FreeStagingBuffer();
	void FreeDeviceBuffer();

private:
	VkBuffer stagingBuffer;
	VkBuffer deviceBuffer;

	VkDeviceMemory stagingMemory;
	VkDeviceMemory nativeMemory;

	VkDeviceSize size;
	BufferType::Enum bufferType;
};
