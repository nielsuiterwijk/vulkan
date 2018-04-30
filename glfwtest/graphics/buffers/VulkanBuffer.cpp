#include "VulkanBuffer.h"

#include "graphics/GraphicsContext.h"
#include "graphics/memory/GPUAllocator.h"

#include <vulkan/vulkan.h>

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags flags, BufferType::Enum bufferType, void* bufferData, size_t size) :
	nativeMemory(nullptr),
	deviceBuffer(nullptr),
	stagingMemory(nullptr),
	stagingBuffer(nullptr),
	size(size),
	bufferType(bufferType)
{

	switch (bufferType)
	{
	case BufferType::Static:
		SetupStagingBuffer(bufferData);

		Map(bufferData);

		SetupLocalStaticBuffer(flags);

		CopyStagingToDevice();
		break;
	case BufferType::Dynamic:
		SetupLocalDynamicBuffer(bufferData, flags);
		Map(bufferData);
		break;

	case BufferType::Staging:
		SetupStagingBuffer(bufferData);
		Map(bufferData);
		break;
	default:
		assert(false && "Not implemented.");
		break;
	}


}

VulkanBuffer::~VulkanBuffer()
{
	GraphicsContext::QueueLock.lock();
	vkQueueWaitIdle(GraphicsContext::GraphicsQueue);
	GraphicsContext::QueueLock.unlock();
	FreeStagingBuffer();
	FreeDeviceBuffer();
}


void VulkanBuffer::FreeStagingBuffer()
{
	if (stagingMemory != nullptr)
	{
		std::cout << "freeing staging buffer of " << Helpers::MemorySizeToString(size) << " bytes" << std::endl;

		vkDestroyBuffer(GraphicsContext::LogicalDevice, stagingBuffer, GraphicsContext::GlobalAllocator.Get());
		vkFreeMemory(GraphicsContext::LogicalDevice, stagingMemory, GraphicsContext::GlobalAllocator.Get());
		stagingMemory = nullptr;
	}
}

void VulkanBuffer::FreeDeviceBuffer()
{
	if (deviceBuffer != nullptr)
	{
		std::cout << "freeing device buffer of " << Helpers::MemorySizeToString(size) << " bytes" << std::endl;

		vkDestroyBuffer(GraphicsContext::LogicalDevice, deviceBuffer, GraphicsContext::GlobalAllocator.Get());
		vkFreeMemory(GraphicsContext::LogicalDevice, nativeMemory, GraphicsContext::GlobalAllocator.Get());
		deviceBuffer = nullptr;
	}
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

}

void VulkanBuffer::CopyStagingToDevice()
{
	auto commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer->GetNative(), stagingBuffer, deviceBuffer, 1, &copyRegion);

	commandBuffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->GetNative();
	

	GraphicsContext::QueueLock.lock();
	//TODO: This causes waits, needs to be async
	vkQueueSubmit(GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsContext::TransportQueue);
	GraphicsContext::QueueLock.unlock();

	GraphicsContext::CommandBufferPoolTransient->Free(commandBuffer);

	FreeStagingBuffer();
}

void VulkanBuffer::CopyStagingToImage(VkImage image, uint32_t width, uint32_t height)
{
	auto commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1	};

	vkCmdCopyBufferToImage(commandBuffer->GetNative(), stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	commandBuffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->GetNative();

	GraphicsContext::QueueLock.lock();
	//TODO: This causes waits, needs to be async
	vkQueueSubmit(GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsContext::TransportQueue);

	GraphicsContext::QueueLock.unlock();

	GraphicsContext::CommandBufferPoolTransient->Free(commandBuffer);

	FreeStagingBuffer();
}

void VulkanBuffer::Map(void* bufferData, uint32_t sizeToMap) const
{
	if (bufferData == nullptr)
		return;

	if (sizeToMap == -1)
		sizeToMap = size;

	assert(sizeToMap <= size);

	VkResult result = VK_SUCCESS;
	void* vulkanVirtualMappedMemoryAddress;

	switch (bufferType)
	{
	case BufferType::Static:
	case BufferType::Staging:
		result = vkMapMemory(GraphicsContext::LogicalDevice, stagingMemory, 0, sizeToMap, 0, &vulkanVirtualMappedMemoryAddress);
		assert(result == VK_SUCCESS);
		memcpy(vulkanVirtualMappedMemoryAddress, bufferData, sizeToMap);
		vkUnmapMemory(GraphicsContext::LogicalDevice, stagingMemory);
		break;
	case BufferType::Dynamic:
		result = vkMapMemory(GraphicsContext::LogicalDevice, nativeMemory, 0, sizeToMap, 0, &vulkanVirtualMappedMemoryAddress);
		assert(result == VK_SUCCESS);
		memcpy(vulkanVirtualMappedMemoryAddress, bufferData, sizeToMap);
		vkUnmapMemory(GraphicsContext::LogicalDevice, nativeMemory);
		break;
	default:
		assert(false && "Not implemented.");
		break;
	}
}
