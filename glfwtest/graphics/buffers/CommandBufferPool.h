#pragma once

#include "graphics\helpers\InstanceWrapper.h"
#include "CommandBuffer.h"
#include "standard.h"

#include <vulkan\vulkan.h>
#include <vector>

class CommandBufferPool
{
public:
	CommandBufferPool();
	~CommandBufferPool();

	//Give it an empty vector and it will fill it with propper Command Buffers
	void Create(std::vector< std::shared_ptr<CommandBuffer> >& result);

	std::shared_ptr<CommandBuffer> Create();

private:
	InstanceWrapper<VkCommandPool> commandPool;
};