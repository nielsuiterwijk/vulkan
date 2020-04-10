#include "UniformBuffer.h"

UniformBuffer::UniformBuffer( DataBlock block ) :
	dataBlock( block ),
	_VulkanBuffer( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, dataBlock.data, dataBlock.size ),
	bufferInfo()
{
	bufferInfo.buffer = _VulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = static_cast<VkDeviceSize>( dataBlock.size );
}
UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::Upload()
{
	_VulkanBuffer.Map( dataBlock.data, dataBlock.size );
}
