#pragma once

#include "standard.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"

class VulkanBuffer;
class Material;
class SubMesh;

struct Vertex 
{
	glm::vec2 pos;
	glm::vec3 color;

	static void GetBindingDescription(VkVertexInputBindingDescription& bindingDescription)
	{
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		attributeDescriptions.resize(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
	}
};

struct Vertex3D
{
	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec3 color;

	static void GetBindingDescription(VkVertexInputBindingDescription& bindingDescription)
	{
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		attributeDescriptions.resize(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, texCoords);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, color);
	}
};

struct VertexPTCN
{
	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec3 color;
	glm::vec3 normal;

	static void GetBindingDescription(VkVertexInputBindingDescription& bindingDescription)
	{
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPTCN);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		attributeDescriptions.resize(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPTCN, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPTCN, texCoords);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexPTCN, color);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(VertexPTCN, normal);
	}

	bool operator==(const VertexPTCN& other) const {
		return pos == other.pos && color == other.color && texCoords == other.texCoords && normal == other.normal;
	}
};

class Mesh
{
public:
	friend class MeshFileLoader;

	struct MeshVertexFormatFlags
	{
		enum Enum
		{
			POSITION = 1 << 0,
			NORMAL = 1 << 1,
			UV = 1 << 2,
			WEIGHTS = 1 << 3,
			TANGENT = 1 << 4,
			BITANGENT = 1 << 5,
			INDEXED = 1 << 6
		};
	};



	explicit Mesh();
	~Mesh();

	SubMesh* AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t triangles);
	void Draw(std::shared_ptr<CommandBuffer> buffer, const PipelineStateObject& pso, std::shared_ptr<Material> material) const;

	const std::vector<SubMesh*>& GetSubMeshes() const { return subMeshes; }

	const VkVertexInputBindingDescription& GetBindingDescription() const { return bindingDescription; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

	bool IsLoaded() const;

private:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::vector<SubMesh*> subMeshes;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};