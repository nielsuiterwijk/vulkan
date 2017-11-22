#include "CommandBufferPool.h"

#include "graphics\GraphicsDevice.h"

CommandBufferPool::CommandBufferPool() :
	commandPool(GraphicsContext::LogicalDevice, vkDestroyCommandPool, GraphicsContext::GlobalAllocator.Get())
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = GraphicsContext::FamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

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

const InstanceWrapper<VkCommandPool>& CommandBufferPool::GetNative() const
{
	return commandPool;
}

/*
VK_COMMAND_BUFFER_LEVEL_PRIMARY:
	Can be submitted to a queue for execution, but cannot be called from other command buffers.
VK_COMMAND_BUFFER_LEVEL_SECONDARY :
	Cannot be submitted directly, but can be called from primary command buffers.
*/

void CommandBufferPool::Create(std::vector< std::shared_ptr<CommandBuffer> >& result, int count)
{
	for (size_t i = 0; i < count; i++)
	{
		result.push_back(Create());
	}
}

std::shared_ptr<CommandBuffer> CommandBufferPool::Create()
{
	std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>();

	commandBuffers.push_back(commandBuffer);

	return commandBuffer;
}

void CommandBufferPool::Clear()
{
	commandBuffers.clear();
}

void CommandBufferPool::Free(std::shared_ptr<CommandBuffer> commandBuffer)
{
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		if (commandBuffers[i] == commandBuffer)
		{
			commandBuffers.erase(commandBuffers.begin() + i);
			break;
		}
	}
}

void CommandBufferPool::FreeAll()
{
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i]->Finalize();
	}
}

void CommandBufferPool::RecreateAll()
{
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i]->Initialize();
	}
}