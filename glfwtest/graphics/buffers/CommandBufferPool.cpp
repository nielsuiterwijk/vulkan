#include "CommandBufferPool.h"

#include "graphics\GraphicsDevice.h"

CommandBufferPool::CommandBufferPool() :
	commandPool(GraphicsContext::LogicalDevice, vkDestroyCommandPool, GraphicsContext::GlobalAllocator.Get())
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = GraphicsContext::FamilyIndices.graphicsFamily;
	poolInfo.flags = 0; // Optional

	/*
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	*/

	if (vkCreateCommandPool(GraphicsContext::LogicalDevice, &poolInfo, GraphicsContext::GlobalAllocator.Get(), commandPool.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

CommandBufferPool::~CommandBufferPool()
{
	commandPool = nullptr;
}

/*
VK_COMMAND_BUFFER_LEVEL_PRIMARY:
	Can be submitted to a queue for execution, but cannot be called from other command buffers.
VK_COMMAND_BUFFER_LEVEL_SECONDARY :
	Cannot be submitted directly, but can be called from primary command buffers.
*/

void CommandBufferPool::Create(std::vector< std::shared_ptr<CommandBuffer> >& result)
{
	//TODO: Assert
	if (result[0] != nullptr)
	{
		throw std::runtime_error("vector wasn't empty!");
	}

	for (size_t i = 0; i < result.size(); i++)
	{
		result[i] = Create();
	}
}

std::shared_ptr<CommandBuffer> CommandBufferPool::Create()
{
	VkCommandBuffer commandBuffer = {};

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(GraphicsContext::LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return std::make_shared<CommandBuffer>(commandBuffer);


}