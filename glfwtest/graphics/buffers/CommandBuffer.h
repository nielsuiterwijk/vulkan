#pragma once

#include <vulkan\vulkan.h>

class CommandBufferPool;

class CommandBuffer
{
public:
	CommandBuffer(VkCommandBuffer commandBuffer);
	~CommandBuffer();

	void StartRecording(int32_t frameIndex);
	void StopRecording();

	const VkCommandBuffer& GetNative() const;

private:
	VkCommandBuffer commandBuffer;
};