#include "UniformBuffer.h"

UniformBuffer::UniformBuffer( DataBlock block ) :
	_DataBlock( block ),
	_VulkanBuffer( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, block.data, block.size ),
	bufferInfo()
{
	bufferInfo.buffer = _VulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = static_cast<VkDeviceSize>( _DataBlock.size );
}
UniformBuffer::~UniformBuffer()
{
	_DataBlock.data = nullptr;
	_DataBlock.size = -1;
}

void UniformBuffer::Upload()
{
	ASSERT( _DataBlock.data != nullptr );
	_VulkanBuffer.Map( _DataBlock.data, _DataBlock.size );
}
