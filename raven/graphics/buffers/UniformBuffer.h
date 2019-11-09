#pragma once

#include "graphics/GraphicsContext.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/buffers/VulkanDescriptorPool.h"


struct DataBlock
{
	void* data = nullptr;
	size_t size = 0;
};

//A small wrapper class that owns the concrete ubo
class UniformBuffer
{
public:
	UniformBuffer( DataBlock data );
	~UniformBuffer();

	void Upload();

	template <class T>
	const T* Get() const
	{
		return static_cast<T*>( dataBlock.data );
	}

	const VkDescriptorBufferInfo& GetDescriptorInfo() const { return bufferInfo; }

private:
	DataBlock dataBlock;

	VulkanBuffer vulkanBuffer;
	VkDescriptorBufferInfo bufferInfo;
};
