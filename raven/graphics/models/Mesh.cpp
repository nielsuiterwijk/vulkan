#include "Mesh.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"

#include "helpers/Timer.h"
#include "tinyobj/tiny_obj_loader.h"

Mesh::Mesh() :
	triangleCount( 0 ),
	vertexFormatFlags( 0 )
{
}

Mesh::~Mesh()
{
	for ( int i = 0; i < subMeshes.size(); i++ )
	{
		delete subMeshes[ i ];
	}

	subMeshes.clear();
}

SubMesh* Mesh::AllocateBuffers( void* vertexData, const uint64_t& vertexDataSize, void* indexData, const uint64_t& indexDataSize, uint32_t triangles )
{
	SubMesh* subMesh = new SubMesh( triangles );

	subMesh->AllocateBuffers( vertexData, vertexDataSize, indexData, indexDataSize );

	subMeshes.emplace_back( subMesh );

	return subMesh;
}

void Mesh::BuildDescriptors( std::shared_ptr<Material> material )
{
	std::shared_ptr<VertexShader> vertexShader = material->GetVertex();
	vertexShader->GetAttributeDescriptions( attributeDescriptions );
	vertexShader->GetBindingDescription( bindingDescription );
}

bool Mesh::IsLoaded() const
{
	for ( int i = 0; i < subMeshes.size(); i++ )
	{
		if ( !subMeshes[ i ]->IsLoaded() )
			return false;
	}

	return true;
}
