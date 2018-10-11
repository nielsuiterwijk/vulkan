#pragma once

#include "standard.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"

class VulkanBuffer;
class Material;
class SubMesh;

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec4 color;
	glm::vec3 normal;

	glm::vec4 joint0;
	glm::vec4 weight0;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoords == other.texCoords && normal == other.normal;
	}

	static size_t GetOffsetFromLocation(int location)
	{
		switch (location)
		{
		case 0:
			return offsetof(Vertex, pos);
		case 1:
			return offsetof(Vertex, texCoords);
		case 2:
			return offsetof(Vertex, color);
		case 3:
			return offsetof(Vertex, normal);
		}

		return -1;
	}
};

class Mesh
{
public:
	friend class MeshFileLoader;
	
	explicit Mesh();
	~Mesh();
	
	const std::vector<SubMesh*>& GetSubMeshes() const { return subMeshes; }

	const VkVertexInputBindingDescription& GetBindingDescription() const { return bindingDescription; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

	void BuildDescriptors(std::shared_ptr<Material> material);

	bool IsLoaded() const;

private:
	SubMesh* AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t triangles);

private:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::vector<SubMesh*> subMeshes;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};