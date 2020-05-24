#pragma once

//#include "graphics\buffers\CommandBufferPool.h"

#include <vulkan\vulkan.h>

class CommandBufferPool;

enum class CommandBufferType
{
	Transient = 0x01,
	AutoReset = 0x02
};

static_assert( (size_t)CommandBufferType::Transient == VK_COMMAND_POOL_CREATE_TRANSIENT_BIT );
static_assert( (size_t)CommandBufferType::AutoReset == VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );

class CommandBuffer
{
public:
	CommandBuffer( VkCommandPool commandBufferPool );
	~CommandBuffer();

	void Initialize();
	void Finalize();

	void StartRecording( VkCommandBufferUsageFlagBits flag );
	void StopRecording();

	const VkCommandBuffer& GetNative() const;

	operator VkCommandBuffer() const { return commandBuffer; }

private:
	VkCommandBuffer commandBuffer;

	VkCommandPool _pCommandBufferPool;
};
