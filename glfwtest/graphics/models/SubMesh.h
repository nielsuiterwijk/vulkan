#pragma once

#include "standard.h"

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

	void SetAABB(glm::vec3 min, glm::vec3 max)
	{
		aabbMin = std::move(min);
		aabbMax = std::move(max);
	}

private:
	uint32_t triangleCount;
	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};