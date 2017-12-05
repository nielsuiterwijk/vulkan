#pragma once

//#include "graphics\buffers\CommandBufferPool.h"

#include "standard.h"

#include <vulkan\vulkan.h>

class CommandBufferPool;

class CommandBuffer
{
public:
	CommandBuffer(std::shared_ptr<CommandBufferPool> commandBufferPool);
	~CommandBuffer();

	void Initialize();
	void Finalize();

	void StartRecording(int32_t frameIndex, VkCommandBufferUsageFlagBits flag);
	void StopRecording();

	const VkCommandBuffer& GetNative() const;

private:
	VkCommandBuffer commandBuffer;

	std::shared_ptr<CommandBufferPool> commandBufferPool;
};