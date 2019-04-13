#pragma once

#include "graphics/PipelineStateObject.h"
#include "graphics/buffers/CommandBuffer.h"
#include "standard.h"

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

	bool operator==( const Vertex& other ) const
	{
		return pos == other.pos && color == other.color && texCoords == other.texCoords && normal == other.normal;
	}

	static size_t GetOffsetFromLocation( int location )
	{
		switch ( location )
		{
		case 0:
			return offsetof( Vertex, pos );
		case 1:
			return offsetof( Vertex, texCoords );
		case 2:
			return offsetof( Vertex, color );
		case 3:
			return offsetof( Vertex, normal );
		case 4:
			return offsetof( Vertex, joint0 );
		case 5:
			return offsetof( Vertex, weight0 );
		}

		return -1;
	}
};

enum class MeshType
{
	Static,
	Skinned
};

class Mesh
{
public:
	friend class MeshFileLoader;

	explicit Mesh();
	virtual ~Mesh();

	const std::vector<SubMesh*>& GetSubMeshes() const { return subMeshes; }

	const VkVertexInputBindingDescription& GetBindingDescription() const { return bindingDescription; }
	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return attributeDescriptions; }

	void BuildDescriptors( std::shared_ptr<Material> material );

	bool IsLoaded() const;

	virtual MeshType GetMeshType() const { return MeshType::Static; }

protected:
	SubMesh* AllocateBuffers( void* vertexData, const size_t& vertexDataSize, void* indexData, const size_t& indexDataSize, uint32_t triangles );

protected:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::vector<SubMesh*> subMeshes;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
};
