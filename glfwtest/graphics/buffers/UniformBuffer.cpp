#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(DataBlock block) :
	dataBlock(block),
	vulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, dataBlock.data, dataBlock.size),
	bufferInfo()
{
	bufferInfo.buffer = vulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = static_cast<VkDeviceSize>(dataBlock.size);
}
UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::Upload()
{
	vulkanBuffer.Map(dataBlock.data);
}