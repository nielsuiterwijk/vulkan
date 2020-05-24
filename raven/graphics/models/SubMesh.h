#pragma once

#include "physics/aabb.h"


class VulkanBuffer;
class CommandBuffer;

class SubMesh
{
public:
	explicit SubMesh();
	SubMesh( uint32_t triangleCount );
	~SubMesh();

	bool AllocateBuffers( void* vertexData, const uint64_t& vertexDataSize, void* indexData, const uint64_t& indexDataSize );
	bool IsLoaded() const { return vertexBuffer != nullptr && indexBuffer != nullptr; }

	void Draw( CommandBuffer& commandBuffer );

	void SetAABB( AABB aabb ) { this->aabb = std::move( aabb ); }

private:
	uint32_t triangleCount;
	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;

	AABB aabb;
};
