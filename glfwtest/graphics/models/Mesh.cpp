#include "Mesh.h"

#include "graphics/buffers/CommandBuffer.h"

Mesh::Mesh(uint32_t triangleCount) :
	triangleCount(triangleCount)
	//indexBuffer(nullptr),
	//vertexBuffer(nullptr)
{
}

Mesh::~Mesh()
{
}

bool Mesh::Initialize(void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t vertexFormat)
{
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

void Mesh::SetupCommandBuffer(const CommandBuffer& buffer) const
{
	/*buffer->BindVertexBuffer(0, vertexBuffer);

	if (vertexFormatFlags & MeshVertexFormatFlags::INDEXED)
	{
		buffer->DrawElements(DrawPrimitive::TRIANGLES, IndexType::UNSIGNED_INT, triangleCount * 3, indexBuffer);
	}
	else
	{
		buffer->DrawArrays(DrawPrimitive::TRIANGLES, 0, triangleCount * 3);
	}*/

}
