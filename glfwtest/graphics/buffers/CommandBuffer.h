#pragma once

#include <vulkan\vulkan.h>

class CommandBuffer
{
public:
	CommandBuffer(VkCommandBuffer commandBuffer);
	~CommandBuffer();

	void StartRecording(int32_t frameIndex);

	void StopRecording();

private:
	VkCommandBuffer commandBuffer;
};