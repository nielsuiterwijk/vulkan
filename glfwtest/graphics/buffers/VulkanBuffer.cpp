#include "VulkanBuffer.h"

#include "graphics/GraphicsDevice.h"

#include <vulkan/vulkan.h>

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags flags, VkSharingMode sharingMode, void* bufferData, size_t size)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = flags;
	bufferInfo.sharingMode = sharingMode;

	if (vkCreateBuffer(GraphicsContext::LogicalDevice, &bufferInfo, nullptr, &nativeBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	


	vkBindBufferMemory(GraphicsContext::LogicalDevice, nativeBuffer, nativeMemory, 0);

	void* data;
	vkMapMemory(GraphicsContext::LogicalDevice , nativeMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, bufferData, (size_t)bufferInfo.size);
	vkUnmapMemory(GraphicsContext::LogicalDevice, nativeMemory);
}

VulkanBuffer::~VulkanBuffer()
{
	vkDestroyBuffer(GraphicsContext::LogicalDevice, nativeBuffer, nullptr);
	vkFreeMemory(GraphicsContext::LogicalDevice, nativeMemory, nullptr);
}