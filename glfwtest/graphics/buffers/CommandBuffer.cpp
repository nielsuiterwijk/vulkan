#include "CommandBuffer.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\VulkanSwapChain.h"
#include "CommandBufferPool.h"

#include "standard.h"


CommandBuffer::CommandBuffer(VkCommandBuffer commandBuffer) :
	commandBuffer(commandBuffer)
{

}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(GraphicsContext::LogicalDevice, GraphicsContext::CommandBufferPool->GetNative(), 1, &commandBuffer);
}

void CommandBuffer::StartRecording(int32_t frameIndex)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
	renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(frameIndex).framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

	VkClearValue clearColor = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void CommandBuffer::StopRecording()
{
	vkCmdEndRenderPass(commandBuffer);


	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

const VkCommandBuffer& CommandBuffer::GetNative() const
{
	return commandBuffer;
}