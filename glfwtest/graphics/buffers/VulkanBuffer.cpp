#include "VulkanBuffer.h"

#include "graphics/GraphicsDevice.h"
#include "graphics/memory/GPUAllocator.h"

#include <vulkan/vulkan.h>

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags flags, VkSharingMode sharingMode, void* bufferData, size_t size) :
	nativeMemory(nullptr),
	nativeBuffer(nullptr)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = flags;
	bufferInfo.sharingMode = sharingMode;

	if (vkCreateBuffer(GraphicsContext::LogicalDevice, &bufferInfo, GraphicsContext::GlobalAllocator.Get(), &nativeBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	
	GraphicsContext::DeviceAllocator->Allocate(*this);

	vkBindBufferMemory(GraphicsContext::LogicalDevice, nativeBuffer, nativeMemory, 0);

	void* data;
	vkMapMemory(GraphicsContext::LogicalDevice, nativeMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, bufferData, (size_t)bufferInfo.size);
	vkUnmapMemory(GraphicsContext::LogicalDevice, nativeMemory);
}

VulkanBuffer::~VulkanBuffer()
{
	vkDestroyBuffer(GraphicsContext::LogicalDevice, nativeBuffer, GraphicsContext::GlobalAllocator.Get());
	vkFreeMemory(GraphicsContext::LogicalDevice, nativeMemory, GraphicsContext::GlobalAllocator.Get());
}