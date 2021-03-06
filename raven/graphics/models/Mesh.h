#pragma once

#include "graphics/PipelineStateObject.h"
#include "graphics/buffers/CommandBuffer.h"

#include "ecs/World.h"


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

	static uint32_t GetOffsetFromLocation( int location )
	{
		switch ( location )
		{
		case 0:
			return static_cast<uint32_t>( offsetof( Vertex, pos ) );
		case 1:
			return static_cast<uint32_t>( offsetof( Vertex, texCoords ) );
		case 2:
			return static_cast<uint32_t>( offsetof( Vertex, color ) );
		case 3:
			return static_cast<uint32_t>( offsetof( Vertex, normal ) );
		case 4:
			return static_cast<uint32_t>( offsetof( Vertex, joint0 ) );
		case 5:
			return static_cast<uint32_t>( offsetof( Vertex, weight0 ) );
		}

		return -1;
	}
};

enum class MeshType
{
	Static,
	Skinned
};

struct MeshComponent
{
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	std::vector<SubMesh*> subMeshes; //todo use shared_ptr as submeshes can be shared between components?

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;


	void BuildDescriptors( const Material* pMaterial );
};

class AABBComponent
{
	glm::vec3 _Min;
	glm::vec3 _Max;
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

	void BuildDescriptors( const Material* pMaterial );

	virtual bool IsLoaded() const;

	virtual MeshType GetMeshType() const { return MeshType::Static; }
	virtual void Assign( Ecs::World& World, Ecs::Entity Entity ) const;

protected:
	SubMesh* AllocateBuffers( void* vertexData, const uint64_t& vertexDataSize, void* indexData, const uint64_t& indexDataSize, uint32_t triangles );

protected:
	uint32_t triangleCount;
	uint32_t vertexFormatFlags;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::vector<SubMesh*> subMeshes;
};
