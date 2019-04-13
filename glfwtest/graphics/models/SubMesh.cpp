#include "SubMesh.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/VulkanBuffer.h"

SubMesh::SubMesh() :
	triangleCount( 0 ),
	indexBuffer( nullptr ),
	vertexBuffer( nullptr )
{
}

SubMesh::SubMesh( uint32_t triangleCount ) :
	triangleCount( triangleCount ),
	indexBuffer( nullptr ),
	vertexBuffer( nullptr )
{
}

SubMesh::~SubMesh()
{
	delete vertexBuffer;
	delete indexBuffer;
}

bool SubMesh::AllocateBuffers( void* vertexData, const uint32_t& vertexDataSize, void* indexData, const uint32_t& indexDataSize )
{
	assert( vertexBuffer == nullptr );
	vertexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static, vertexData, vertexDataSize );

	assert( indexBuffer == nullptr );
	indexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Static, indexData, indexDataSize );

	return true;
}

void SubMesh::Draw( std::shared_ptr<CommandBuffer> commandBuffer )
{
	VkBuffer vertexBuffers[] = { vertexBuffer->GetNative() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer->GetNative(), 0, 1, vertexBuffers, offsets );

	vkCmdBindIndexBuffer( commandBuffer->GetNative(), indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32 );

	vkCmdDrawIndexed( commandBuffer->GetNative(), triangleCount * 3, 1, 0, 0, 0 );
}
