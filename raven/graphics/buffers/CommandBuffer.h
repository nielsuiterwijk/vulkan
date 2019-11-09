#pragma once

//#include "graphics\buffers\CommandBufferPool.h"

#include <vulkan\vulkan.h>

class CommandBufferPool;

class CommandBuffer
{
public:
	CommandBuffer( CommandBufferPool* commandBufferPool );
	~CommandBuffer();

	void Initialize();
	void Finalize();

	void StartRecording( VkCommandBufferUsageFlagBits flag );
	void StopRecording();

	const VkCommandBuffer& GetNative() const;

private:
	VkCommandBuffer commandBuffer;

	CommandBufferPool* commandBufferPool;
};
