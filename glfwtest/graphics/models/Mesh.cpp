#include "Mesh.h"

#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/shaders/Material.h"
#include "graphics/buffers/UniformBuffer.h"

#include "helpers/Timer.h"
#include "tinyobj/tiny_obj_loader.h"

Mesh::Mesh() :
	triangleCount(0),
	indexBuffer(nullptr),
	vertexBuffer(nullptr)
{
	const std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	Vertex::GetBindingDescription(bindingDescription);
	Vertex::GetAttributeDescriptions(attributeDescriptions);
	
	uint32_t memorySize = sizeof(Vertex) * vertices.size();
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);

	uint32_t memorySizeIndices = sizeof(uint16_t) * indices.size();
	uint8_t* indexData = new uint8_t[memorySizeIndices];
	memcpy(indexData, indices.data(), memorySizeIndices);

	triangleCount = indices.size() / 3;
	Initialize(vertexData, memorySize, indexData, memorySizeIndices);

	delete [] vertexData;
	delete [] indexData;
}

Mesh::Mesh(const std::string& fileName) :
	triangleCount(0),
	indexBuffer(nullptr),
	vertexBuffer(nullptr)
{
	std::vector<Vertex3D> vertices;
	std::vector<uint32_t> indices;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	Timer loadTimer;
	loadTimer.Start();
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str())) 
	{
		throw std::runtime_error(err);
	}
	loadTimer.Stop();
	std::cout << "LoadObj took: " << loadTimer.GetTimeInSeconds() << " seconds." << std::endl;

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex3D vertex = {};

			vertex.pos = 
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			/*vertex.texCoords = 
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1]
			};*/

			vertex.color = 
			{ 
				1.0f, 1.0f, 1.0f
				/*attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]*/
			};

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}

	triangleCount = indices.size() / 3;

	for (size_t i = 0; i < triangleCount; i++)
	{
		Vertex3D v1 = vertices[3 * i + 0];
		Vertex3D v2 = vertices[3 * i + 1];
		Vertex3D v3 = vertices[3 * i + 2];

		glm::vec3 edge1 = v1.pos - v2.pos;
		glm::vec3 edge2 = v1.pos - v3.pos;
		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		vertices[3 * i + 0].color = normal;
		vertices[3 * i + 1].color = normal;
		vertices[3 * i + 2].color = normal;
	}

	loadTimer.Stop();
	std::cout << "Allocating vertices & indices took: " << loadTimer.GetTimeInSeconds() << " seconds." << std::endl;

	Vertex3D::GetBindingDescription(bindingDescription);
	Vertex3D::GetAttributeDescriptions(attributeDescriptions);

	uint32_t memorySize = sizeof(Vertex3D) * vertices.size();
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);

	uint32_t memorySizeIndices = sizeof(uint32_t) * indices.size();
	uint8_t* indexData = new uint8_t[memorySizeIndices];
	memcpy(indexData, indices.data(), memorySizeIndices);

	Initialize(vertexData, memorySize, indexData, memorySizeIndices);
}

Mesh::~Mesh()
{
	delete vertexBuffer;
	delete indexBuffer;
}

bool Mesh::Initialize(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize)
{
	assert(vertexBuffer == nullptr);
	vertexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static , vertexData, vertexDataSize);

	assert(indexBuffer == nullptr);
	indexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Static, indexData, indexDataSize);

	return true;
}

void Mesh::SetupCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer, const PipelineStateObject& pso, std::shared_ptr<Material> material) const
{	
	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());

	VkBuffer vertexBuffers[] = { vertexBuffer->GetNative() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetNative(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer->GetNative(), indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);
		
	
	vkCmdBindDescriptorSets(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetLayout(), 0, 1, &material->GetUniformBuffers()[0]->GetDescriptorSet(), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer->GetNative(), triangleCount * 3, 1, 0, 0, 0);

}
