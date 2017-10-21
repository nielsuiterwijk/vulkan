#pragma once

#include "standard.h"
#include "graphics\buffers\CommandBuffer.h"

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
	void SetupCommandBuffer(const CommandBuffer& buffer) const;

private:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	//Buffer* indexBuffer;
	//Buffer* vertexBuffer;
};