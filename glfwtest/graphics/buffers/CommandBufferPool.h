#pragma once

#include "graphics\helpers\InstanceWrapper.h"
#include "CommandBuffer.h"
#include "standard.h"

#include <vulkan\vulkan.h>
#include <vector>

class CommandBufferPool
{
public:
	CommandBufferPool(VkCommandPoolCreateFlags createFlags);
	~CommandBufferPool();

	//Give it an empty vector and it will fill it with propper Command Buffers
	void Create(std::vector< std::shared_ptr<CommandBuffer> >& result, int count);

	std::shared_ptr<CommandBuffer> Create();

	void Clear();
	void Free(std::shared_ptr<CommandBuffer> commandBuffer);

	void FreeAll();
	void RecreateAll();

	const InstanceWrapper<VkCommandPool>& GetNative() const;

private:
	InstanceWrapper<VkCommandPool> commandPool;

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
};