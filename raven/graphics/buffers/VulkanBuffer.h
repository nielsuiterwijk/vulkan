#pragma once

#include <vulkan/vulkan.h>
#include "graphics/memory/GPUAllocator.h"

enum class BufferType
{
	Static, //Upload & forget
	Dynamic, //Buffers that regularly get updated from the gpu, keeps a CPU shadow copy
	Staging //Used for uploading, for example, pixels to the gpu but then transition those into an image buffer.
};

class VulkanBuffer
{
public:
	VulkanBuffer( VkBufferUsageFlags flags, BufferType bufferType, void* data, VkDeviceSize size );
	~VulkanBuffer();

	const VkBuffer& GetNative() const { return _NativeBuffer.Buffer; }

	VkDeviceSize GetSize() const { return _Size; }
	BufferType GetType() const { return _BufferType; }

	void CopyToBufferAndClear(VulkanBuffer& Destination);
	void CopyToImageAndClear( VkImage image, uint32_t width, uint32_t height );

	void Map( void* bufferData, VkDeviceSize sizeToMap = -1 ) const;

private:
	void SetupStagingBuffer();
	void SetupLocalStaticBuffer( VkBufferUsageFlags flags );

	void SetupLocalDynamicBuffer( VkBufferUsageFlags flags );

	void Free();

private:
	SAllocatedBuffer _NativeBuffer;

	VkDeviceSize _Size;
	BufferType _BufferType;
};
