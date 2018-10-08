#pragma once

#include "Shader.h"
#include "graphics/models/Mesh.h"
#include <atomic>

class VertexShader : public Shader
{
public:
	VertexShader(const std::string& fileName);
	virtual ~VertexShader();

	virtual bool IsLoaded() const override { return filesLoaded.load(std::memory_order_relaxed) == 2; }

	void GetBindingDescription(VkVertexInputBindingDescription& bindingDescription)
	{
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		attributeDescriptions.resize(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, texCoords);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, color);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);
	}

private:
	void ShaderLoaded(std::vector<char> fileData);
	void MetaLoaded(std::vector<char> fileData);

private:
	std::atomic_int32_t filesLoaded;
};