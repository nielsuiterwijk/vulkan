#include "Mesh.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"

#include "helpers/Timer.h"
#include "tinyobj/tiny_obj_loader.h"


void MeshComponent::BuildDescriptors( std::shared_ptr<Material> material )
{
	const VertexShader* vertexShader = material->GetVertex();
	vertexShader->FillAttributeDescriptions( attributeDescriptions );
	vertexShader->FillBindingDescription( bindingDescription );
}

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

void Mesh::BuildDescriptors( Material* material )
{
	const VertexShader* vertexShader = material->GetVertex();
	vertexShader->FillAttributeDescriptions( attributeDescriptions );
	vertexShader->FillBindingDescription( bindingDescription );
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
