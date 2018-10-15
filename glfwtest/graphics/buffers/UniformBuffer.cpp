#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(std::shared_ptr<void> data, uint32_t size) :
	data(data),
	size(size),
	vulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, data.get(), size),
	bufferInfo()
{
	bufferInfo.buffer = vulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = size;

}

UniformBuffer::~UniformBuffer()
{
	data = nullptr;
	size = 0;
}

void UniformBuffer::Upload()
{
	vulkanBuffer.Map(data.get());
}