#pragma once

#include "standard.h"
#include "graphics/buffers/CommandBuffer.h"
#include "graphics/PipelineStateObject.h"

class VulkanBuffer;

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

class Mesh
{
public:
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

	Mesh(uint32_t triangleCount);
	~Mesh();

	bool Initialize(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t vertexFormat);
	void SetupCommandBuffer(std::shared_ptr<CommandBuffer> buffer, const PipelineStateObject& pso) const;

	const VkVertexInputBindingDescription& GetBindingDescription() const { return bindingDescription; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

private:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;
};