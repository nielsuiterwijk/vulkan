#pragma once

#include "standard.h"
#include "physics/aabb.h"

class VulkanBuffer;
class CommandBuffer;

class SubMesh
{
public:
	explicit SubMesh();
	SubMesh(uint32_t triangleCount);
	~SubMesh();

	bool AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize);	
	bool IsLoaded() const { return vertexBuffer != nullptr && indexBuffer != nullptr; }

	void Draw(std::shared_ptr<CommandBuffer> commandBuffer);


	
	void SetAABB(AABB aabb)
	{
		this->aabb = std::move(aabb);
	}

private:
	uint32_t triangleCount;
	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;

	AABB aabb;
};