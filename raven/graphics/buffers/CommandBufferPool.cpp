#include "CommandBufferPool.h"

#include "graphics\GraphicsContext.h"
#include "graphics\GraphicsDevice.h"


thread_local VkCommandPool tl_CommandPool = nullptr;

CommandBufferPool::CommandBufferPool( VkCommandPoolCreateFlags createFlags ) :
	_CreateFlags( createFlags )
{
	
}

CommandBufferPool::~CommandBufferPool()
{
	for ( VkCommandPool Pool : _AllCommandPools )
	{
		vkDestroyCommandPool( GraphicsContext::LogicalDevice, Pool, GraphicsContext::GlobalAllocator.Get() );
	}
}

VkCommandPool CommandBufferPool::AccessOrCreateCommandPool()
{
	if ( tl_CommandPool == nullptr )
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = GraphicsContext::FamilyIndices.graphicsFamily;
		poolInfo.flags = _CreateFlags; // Optional

		/*
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		*/

		if ( vkCreateCommandPool( GraphicsContext::LogicalDevice, &poolInfo, GraphicsContext::GlobalAllocator.Get(), &tl_CommandPool ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create command pool!" );
		}
		else
		{
			std::scoped_lock<Mutex> Lock( _Mutex );

			_AllCommandPools.push_back( tl_CommandPool );
		}

	}

	return tl_CommandPool;
}

VkCommandPool CommandBufferPool::GetNative() const
{
	assert( tl_CommandPool != nullptr );
	return tl_CommandPool;
}

/*
VK_COMMAND_BUFFER_LEVEL_PRIMARY:
	Can be submitted to a queue for execution, but cannot be called from other command buffers.
VK_COMMAND_BUFFER_LEVEL_SECONDARY :
	Cannot be submitted directly, but can be called from primary command buffers.
*/

void CommandBufferPool::Create( std::vector<CommandBuffer*>& result, int count )
{
	result.reserve( count );
	for ( size_t i = 0; i < count; i++ )
	{
		result.push_back( Create() );
	}
}

CommandBuffer* CommandBufferPool::Create()
{
	AccessOrCreateCommandPool();
	//Note: needs to be synchronized as it can be called from multiple threads.
	std::scoped_lock<Mutex> Lock( _Mutex );
	commandBuffers.emplace_back( std::make_unique<CommandBuffer>( this ) );

	return commandBuffers.back().get();
}

void CommandBufferPool::Clear()
{
	std::scoped_lock<Mutex> Lock( _Mutex );
	commandBuffers.clear();
}
//TODO: Fix this coupling, its ugly.
void CommandBufferPool::Free( CommandBuffer* commandBuffer )
{
	std::scoped_lock<Mutex> Lock( _Mutex );
	for ( size_t i = 0; i < commandBuffers.size(); i++ )
	{
		if ( commandBuffers[ i ].get() == commandBuffer )
		{
			commandBuffers.erase( commandBuffers.begin() + i );
			break;
		}
	}
}

void CommandBufferPool::FreeAll()
{
	std::scoped_lock<Mutex> Lock( _Mutex );
	for ( size_t i = 0; i < commandBuffers.size(); i++ )
	{
		commandBuffers[ i ]->Finalize();
	}
}

void CommandBufferPool::RecreateAll()
{
	std::scoped_lock<Mutex> Lock( _Mutex );
	for ( size_t i = 0; i < commandBuffers.size(); i++ )
	{
		commandBuffers[ i ]->Initialize();
	}
}
