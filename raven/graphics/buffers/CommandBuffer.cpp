#include "CommandBuffer.h"
#include "graphics\GraphicsContext.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\buffers\CommandBufferPool.h"



CommandBuffer::CommandBuffer( CommandBufferPool* commandBufferPool ) :
	_pCommandBufferPool( commandBufferPool )
{
	
	Initialize();
}

CommandBuffer::~CommandBuffer()
{
	Finalize();
	_pCommandBufferPool = nullptr;
}

void CommandBuffer::Initialize()
{
	ASSERT( _pCommandBufferPool != nullptr );
	ASSERT( _pCommandBufferPool->GetNative() != nullptr );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _pCommandBufferPool->GetNative();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if ( vkAllocateCommandBuffers( GraphicsContext::LogicalDevice, &allocInfo, &commandBuffer ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to allocate command buffers!" );
	}
}

void CommandBuffer::Finalize()
{
	vkFreeCommandBuffers( GraphicsContext::LogicalDevice, _pCommandBufferPool->GetNative(), 1, &commandBuffer );
}

void CommandBuffer::StartRecording( VkCommandBufferUsageFlagBits flag )
{
	//TODO: add error checking for the different states?
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flag;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vkBeginCommandBuffer( commandBuffer, &beginInfo );
}

void CommandBuffer::StopRecording()
{

	if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to record command buffer!" );
	}
}

const VkCommandBuffer& CommandBuffer::GetNative() const
{
	return commandBuffer;
}
