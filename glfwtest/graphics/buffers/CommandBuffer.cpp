#include "CommandBuffer.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\buffers\CommandBufferPool.h"
#include "graphics\GraphicsContext.h"

#include "standard.h"


CommandBuffer::CommandBuffer(std::shared_ptr<CommandBufferPool> commandBufferPool) :
	commandBufferPool(commandBufferPool)
{
	Initialize();
}

CommandBuffer::~CommandBuffer()
{
	Finalize();
	commandBufferPool = nullptr;
}

void CommandBuffer::Initialize()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandBufferPool->GetNative();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(GraphicsContext::LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void CommandBuffer::Finalize()
{
	vkFreeCommandBuffers(GraphicsContext::LogicalDevice, commandBufferPool->GetNative(), 1, &commandBuffer);
}

void CommandBuffer::StartRecording(int32_t frameIndex, VkCommandBufferUsageFlagBits flag)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flag;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

}

void CommandBuffer::StopRecording()
{

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

const VkCommandBuffer& CommandBuffer::GetNative() const
{
	return commandBuffer;
}