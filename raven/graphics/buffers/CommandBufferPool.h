#pragma once

#include "graphics\buffers\CommandBuffer.h"
#include "graphics\helpers\InstanceWrapper.h"

//public std::enable_shared_from_this<Foo>
class CommandBufferPool : public std::enable_shared_from_this<CommandBufferPool>
{
public:
	CommandBufferPool( VkCommandPoolCreateFlags createFlags );
	~CommandBufferPool();

	//Give it an empty vector and it will fill it with proper Command Buffers
	void Create( std::vector<CommandBuffer*>& result, int count );

	CommandBuffer* Create();

	void Clear();
	void Free( CommandBuffer* commandBuffer );

	void FreeAll();
	void RecreateAll();

	const InstanceWrapper<VkCommandPool>& GetNative() const;

private:
	InstanceWrapper<VkCommandPool> commandPool;

	std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
};
