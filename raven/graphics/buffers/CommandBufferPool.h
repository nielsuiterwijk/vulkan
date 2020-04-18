#pragma once

#include "graphics\buffers\CommandBuffer.h"
#include "graphics\helpers\InstanceWrapper.h"
#include "threading\Mutex.h"


/*
This object stays alive until end of program. Internally it contains VkCommandBufferPool objects, one per thread using thread_local storage.
In it's deconstructor it will clean up the created objects, the thread_local objects will keep pointing to, now invalid, VkCommandBufferPool objects
*/
class CommandBufferPool
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

	VkCommandPool GetNative() const;

private:
	//This function will setup the thread_local command pool
	VkCommandPool AccessOrCreateCommandPool();

private:
	VkCommandPoolCreateFlags _CreateFlags;
	Mutex _Mutex;
	std::vector<VkCommandPool> _AllCommandPools;

	std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
};
