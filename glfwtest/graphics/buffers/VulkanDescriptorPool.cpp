#include "VulkanDescriptorPool.h"

#include "graphics/GraphicsContext.h"


VulkanDescriptorPool::VulkanDescriptorPool(uint32_t count)
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = count;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1; //number of pools
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = count;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(GraphicsContext::LogicalDevice, &poolInfo, GraphicsContext::GlobalAllocator.Get(), &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	vkDestroyDescriptorPool(GraphicsContext::LogicalDevice, descriptorPool, GraphicsContext::GlobalAllocator.Get());
}

const VkDescriptorPool& VulkanDescriptorPool::Get() const
{
	return descriptorPool;
}