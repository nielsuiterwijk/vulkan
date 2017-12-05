#include "Mesh.h"

#include "graphics/buffers/VulkanBuffer.h"

Mesh::Mesh(uint32_t newTriangleCount) :
	triangleCount(newTriangleCount),
	//indexBuffer(nullptr),
	vertexBuffer(nullptr)
{
	const std::vector<Vertex> vertices =
	{
		{ { 0.0f, -0.8f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.8f },{ 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { 0.0f, -0.8f },{ 1.0f, 0.0f, 0.0f } }
	};
	/*const std::vector<Vertex> vertices =
	{
		{ { 0.0f, -0.8f },{ 1.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
	};*/

	Vertex::GetBindingDescription(bindingDescription);
	Vertex::GetAttributeDescriptions(attributeDescriptions);

	triangleCount = vertices.size();
	uint32_t memorySize = sizeof(Vertex) * triangleCount;
	uint8_t* vertexData = new uint8_t[memorySize];
	memcpy(vertexData, vertices.data(), memorySize);
	Initialize(vertexData, memorySize, nullptr, 0, 0);
}

Mesh::~Mesh()
{
}

bool Mesh::Initialize(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t vertexFormat)
{
	assert(vertexBuffer == nullptr);
	vertexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static , vertexData, vertexDataSize);

	/*if (vertexFormat & MeshVertexFormatFlags::INDEXED)
	{
		if (indexData == nullptr)
		{
			std::cout << "Missing index data!" << std::endl;
			return false;
		}

		indexBuffer = new Buffer(indexData, indexDataSize, Buffer::BufferType::INDEX, GraphicsMemoryPool::MemoryFlags::CPU_NO_ACCESS | GraphicsMemoryPool::MemoryFlags::GPU_CACHED, nullptr);
	}

	vertexBuffer = new Buffer(vertexData, vertexDataSize, Buffer::BufferType::VERTEX, GraphicsMemoryPool::MemoryFlags::CPU_NO_ACCESS | GraphicsMemoryPool::MemoryFlags::GPU_CACHED, nullptr);

	vertexFormatFlags = vertexFormat;*/

	return true;
}

void Mesh::SetupCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer, const PipelineStateObject& pso) const
{	
	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());

	VkBuffer vertexBuffers[] = { vertexBuffer->GetNative() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetNative(), 0, 1, vertexBuffers, offsets);

	vkCmdDraw(commandBuffer->GetNative(), triangleCount, 1, 0, 0);

}
