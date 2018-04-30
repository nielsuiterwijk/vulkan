#pragma once

#include "standard.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"

class VulkanBuffer;
class Material;

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

	bool AllocateBuffers(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize);
	void SetupCommandBuffer(std::shared_ptr<CommandBuffer> buffer, const PipelineStateObject& pso, std::shared_ptr<Material> material) const;

	const VkVertexInputBindingDescription& GetBindingDescription() const { return bindingDescription; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

	bool IsLoaded() const { return vertexBuffer != nullptr && indexBuffer != nullptr; }

private:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};