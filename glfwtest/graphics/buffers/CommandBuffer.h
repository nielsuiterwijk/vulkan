#pragma once

#include <vulkan\vulkan.h>

class CommandBuffer
{
public:
	CommandBuffer(VkCommandBuffer commandBuffer);
	~CommandBuffer();

	void StartRecording();

	void StopRecording();

private:
	VkCommandBuffer commandBuffer;
};