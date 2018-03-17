#include "Mesh.h"

#include "graphics/buffers/VulkanBuffer.h"

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
	Initialize(vertexData, memorySize, indexData, memorySizeIndices, 0);

	delete [] vertexData;
	delete [] indexData;
}

Mesh::~Mesh()
{
	delete vertexBuffer;
	delete indexBuffer;
}

bool Mesh::Initialize(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t vertexFormat)
{
	assert(vertexBuffer == nullptr);
	vertexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Static , vertexData, vertexDataSize);

	assert(indexBuffer == nullptr);
	indexBuffer = new VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Dynamic, indexData, indexDataSize);

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

	vkCmdBindIndexBuffer(commandBuffer->GetNative(), indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT16);

	//vkCmdDraw(commandBuffer->GetNative(), triangleCount, 1, 0, 0);
	vkCmdDrawIndexed(commandBuffer->GetNative(), triangleCount * 3, 1, 0, 0, 0);

}
