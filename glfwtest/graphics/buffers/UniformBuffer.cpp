#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(void* data, size_t size) : 
	data(data),
	size(size),
	vulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, data, size),
	bufferInfo()
{
	bufferInfo.buffer = vulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = size;

}

UniformBuffer::~UniformBuffer()
{
	delete[] data;
	data = nullptr;
}

void UniformBuffer::Upload()
{
	vulkanBuffer.Map(data);
}