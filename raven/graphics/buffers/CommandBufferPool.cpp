#include "CommandBufferPool.h"

#include "graphics\GraphicsContext.h"
#include "graphics\GraphicsDevice.h"


//Kinda hard coded atm.
constexpr int32_t TypesSupported = 2;

VkCommandPool& AccessThreadLocal( CommandBufferType Type )
{
	thread_local VkCommandPool tl_CommandPoolPerType[ TypesSupported ];

	int32_t Index = Utility::ctz( static_cast<uint32_t>( Type ) );
	return tl_CommandPoolPerType[ Index ];
}
#define var my_var()


/*
	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
	VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
*/

CommandBufferPool::CommandBufferPool( int32_t PoolsRequired )
{
	_AllCommandPools.reserve( TypesSupported );

	for ( int32_t i = 0; i < TypesSupported; i++ )
	{
		VkCommandPoolCreateFlags CreateFlags = 1 << i;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = GraphicsContext::FamilyIndices.graphicsFamily;
		poolInfo.flags = CreateFlags;

		_AllCommandPools.push_back( {} );
		_AllCommandPools[ i ].reserve( PoolsRequired );

		for ( int32_t j = 0; j < PoolsRequired; j++ )
		{

			VkCommandPool pCommandPool;
			if ( vkCreateCommandPool( GraphicsContext::LogicalDevice, &poolInfo, GraphicsContext::LocalAllocator, &pCommandPool ) != VK_SUCCESS )
			{
				ASSERT_FAIL( "failed to create command pool!" );
			}
			else
			{
				_AllCommandPools[ i ].push_back( { pCommandPool, poolInfo } );
			}
		}
	}

	_FreeCommandPools = _AllCommandPools;
}

CommandBufferPool::~CommandBufferPool()
{
	_FreeCommandPools.clear();

	for ( auto& PoolPerType : _AllCommandPools )
	{
		for ( auto& PoolInfo : PoolPerType )
		{
			vkDestroyCommandPool( GraphicsContext::LogicalDevice, PoolInfo._pPool, GraphicsContext::LocalAllocator );
		}
	}
}

VkCommandPool CommandBufferPool::AccessOrCreateCommandPool( CommandBufferType Type )
{
	if ( AccessThreadLocal( Type ) == nullptr )
	{
		std::scoped_lock<Mutex> Lock( _Mutex );

		int32_t Index = Utility::ctz( static_cast<uint32_t>( Type ) );
		auto& Free = _FreeCommandPools[ Index ];

		if ( Free.empty() )
		{
			ASSERT_FAIL( "Not enough command pools allocated" );
			return VK_NULL_HANDLE;
		}

		AccessThreadLocal( Type ) = Utility::PopBack( Free )._pPool;
	}

	return AccessThreadLocal( Type );
}

VkCommandPool CommandBufferPool::GetNative( CommandBufferType Type ) const
{
	ASSERT( AccessThreadLocal( Type ) != nullptr );
	return AccessThreadLocal( Type );
}

/*
VK_COMMAND_BUFFER_LEVEL_PRIMARY:
	Can be submitted to a queue for execution, but cannot be called from other command buffers.
VK_COMMAND_BUFFER_LEVEL_SECONDARY :
	Cannot be submitted directly, but can be called from primary command buffers.
*/

void CommandBufferPool::Create( std::vector<CommandBuffer*>& result, int count, CommandBufferType Type )
{
	result.reserve( count );
	for ( size_t i = 0; i < count; i++ )
	{
		result.push_back( Create( Type ) );
	}
}

CommandBuffer* CommandBufferPool::Create( CommandBufferType Type )
{
	VkCommandPool ThreadLocalPool = AccessOrCreateCommandPool( Type );
	//Note: needs to be synchronized as it can be called from multiple threads.
	std::scoped_lock<Mutex> Lock( _Mutex );
	commandBuffers.emplace_back( std::make_unique<CommandBuffer>( ThreadLocalPool ) );

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
