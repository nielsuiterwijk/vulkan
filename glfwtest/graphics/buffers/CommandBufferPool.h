#pragma once

#include "standard.h"
#include "graphics\helpers\InstanceWrapper.h"
#include "graphics\buffers\CommandBuffer.h"

//public std::enable_shared_from_this<Foo>
class CommandBufferPool : public std::enable_shared_from_this<CommandBufferPool>
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

	std::vector< std::shared_ptr<CommandBuffer> > commandBuffers;
};