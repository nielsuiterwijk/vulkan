#include "VulkanBuffer.h"

#include "graphics/GraphicsDevice.h"
#include "graphics/memory/GPUAllocator.h"

#include <vulkan/vulkan.h>

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags flags, BufferType::Enum bufferType, void* bufferData, size_t size) :
	nativeMemory(nullptr),
	deviceBuffer(nullptr),
	stagingMemory(nullptr),
	stagingBuffer(nullptr),
	size(size)
{
	//TODO: Dynamic memory might not want to use the staging buffer?
	switch (bufferType)
	{
	case BufferType::Static:
		SetupStagingBuffer(bufferData);

		SetupLocalStaticBuffer(flags);

		CopyStagingToDevice();
		break;
	case BufferType::Dynamic:
		SetupLocalDynamicBuffer(bufferData, flags);
		break;
	default:
		break;
	}


}

VulkanBuffer::~VulkanBuffer()
{
	if (stagingMemory != nullptr)
	{
		vkDestroyBuffer(GraphicsContext::LogicalDevice, stagingBuffer, GraphicsContext::GlobalAllocator.Get());
		vkFreeMemory(GraphicsContext::LogicalDevice, stagingMemory, GraphicsContext::GlobalAllocator.Get());
	}

	vkDestroyBuffer(GraphicsContext::LogicalDevice, deviceBuffer, GraphicsContext::GlobalAllocator.Get());
	vkFreeMemory(GraphicsContext::LogicalDevice, nativeMemory, GraphicsContext::GlobalAllocator.Get());
}


void VulkanBuffer::SetupLocalDynamicBuffer(void* bufferData, VkBufferUsageFlags flags)
{
	VkBufferCreateInfo localBufferInfo = {};
	localBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	localBufferInfo.size = size;
	localBufferInfo.usage = flags;
	localBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GraphicsContext::LogicalDevice, &localBufferInfo, GraphicsContext::GlobalAllocator.Get(), &deviceBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	GraphicsContext::DeviceAllocator->Allocate(deviceBuffer, &nativeMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkBindBufferMemory(GraphicsContext::LogicalDevice, deviceBuffer, nativeMemory, 0);

	void* data;
	vkMapMemory(GraphicsContext::LogicalDevice, nativeMemory, 0, size, 0, &data);
	memcpy(data, bufferData, size);
	vkUnmapMemory(GraphicsContext::LogicalDevice, nativeMemory);
}

void VulkanBuffer::SetupLocalStaticBuffer(VkBufferUsageFlags flags)
{
	VkBufferCreateInfo localBufferInfo = {};
	localBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	localBufferInfo.size = size;
	localBufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	localBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GraphicsContext::LogicalDevice, &localBufferInfo, GraphicsContext::GlobalAllocator.Get(), &deviceBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	GraphicsContext::DeviceAllocator->Allocate(deviceBuffer, &nativeMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkBindBufferMemory(GraphicsContext::LogicalDevice, deviceBuffer, nativeMemory, 0);
}

void VulkanBuffer::SetupStagingBuffer(void* bufferData)
{
	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GraphicsContext::LogicalDevice, &stagingBufferInfo, GraphicsContext::GlobalAllocator.Get(), &stagingBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	GraphicsContext::DeviceAllocator->Allocate(stagingBuffer, &stagingMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkBindBufferMemory(GraphicsContext::LogicalDevice, stagingBuffer, stagingMemory, 0);

	void* data;
	vkMapMemory(GraphicsContext::LogicalDevice, stagingMemory, 0, size, 0, &data);
	memcpy(data, bufferData, size);
	vkUnmapMemory(GraphicsContext::LogicalDevice, stagingMemory);
}

void VulkanBuffer::CopyStagingToDevice()
{
	auto buffer = GraphicsContext::CommandBufferPoolTransient->Create();

	buffer->StartRecording(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(buffer->GetNative(), stagingBuffer, deviceBuffer, 1, &copyRegion);

	buffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer->GetNative();

	vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsContext::GraphicsQueue);

	GraphicsContext::CommandBufferPoolTransient->Free(buffer);
}
