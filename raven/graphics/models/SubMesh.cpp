#include "SubMesh.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/VulkanBuffer.h"

SubMesh::SubMesh()
	: triangleCount( 0 )
	, indexBuffer( nullptr )
	, vertexBuffer( nullptr )
{
}

SubMesh::SubMesh( uint32_t triangleCount )
	: triangleCount( triangleCount )
	, indexBuffer( nullptr )
	, vertexBuffer( nullptr )
{
}

SubMesh::~SubMesh()
{
	delete vertexBuffer;
	delete indexBuffer;
}

bool SubMesh::AllocateBuffers( void* vertexData, const uint64_t& vertexDataSize, void* indexData, const uint64_t& indexDataSize )
{
	ASSERT( vertexBuffer == nullptr );
	vertexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static, vertexData, vertexDataSize );

	ASSERT( indexBuffer == nullptr );
	indexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Static, indexData, indexDataSize );

	return true;
}

void SubMesh::Draw( CommandBuffer& commandBuffer )
{
	VkBuffer vertexBuffers[] = { vertexBuffer->GetNative() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, vertexBuffers, offsets );

	vkCmdBindIndexBuffer( commandBuffer, indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32 );

	vkCmdDrawIndexed( commandBuffer, triangleCount * 3, 1, 0, 0, 0 );
}
