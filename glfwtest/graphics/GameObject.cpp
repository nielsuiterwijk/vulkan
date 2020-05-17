#include "GameObject.h"

#include "io/FileSystem.h"

#include "graphics/models/MeshFileLoader.h"
#include "graphics/textures/TextureLoader.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/models/SkinnedMesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSampler.h"
#include "graphics/PipelineStateCache.h"

GameObject::GameObject( const std::string& objectFile )
	: material( nullptr )
	, mesh( nullptr )
{
	FileSystem::LoadFileAsync( "gameobjects/" + objectFile + ".gameobject", std::bind( &GameObject::FileLoaded, this, std::placeholders::_1 ) );
}

GameObject::~GameObject()
{
}

void GameObject::FileLoaded( std::vector<char> fileData )
{
	std::string fileContents( fileData.data(), fileData.size() );

	auto jsonObject = json::parse( fileContents );

	std::string meshFileName = jsonObject[ "mesh" ];
	std::string materialFileName = jsonObject[ "material" ];
	
	mesh = MeshFileLoader::Dynamic( meshFileName );
	material = std::make_shared<Material>( materialFileName );
	material->AddUniformBuffer( new UniformBuffer( { static_cast<void*>( &camera ), sizeof( Camera::Buffer ) } ) );
	  
	std::vector<std::string> texturesJson = jsonObject[ "textures" ];

	std::vector<std::string> textureFileNames;

	for ( std::string textureFileName : texturesJson )
	{
		bool alreadyAdded = false;

		for ( int j = 0; j < textureFileNames.size(); j++ )
		{
			if ( textureFileNames[ j ] == textureFileName )
			{
				textureFileNames.push_back( textureFileName );
				textures.push_back( textures[ j ] );
				alreadyAdded = true;
				break;
			}
		}

		if ( !alreadyAdded )
		{
			textureFileNames.push_back( textureFileName );
			textures.push_back( TextureLoader::Get( textureFileName ) );
		}
	}
}

#include "ecs/World.h"

Ecs::Entity GameObject::CreateInstance( Ecs::World& World, int32_t Count )
{
	ASSERT( Count == 1 );
	//ASSERT( material->GetUniformBuffers().size() == 0 );

	if ( material == nullptr )
		ASSERT_FAIL("GameObject withour material");
	if ( mesh == nullptr )
		ASSERT_FAIL( "GameObject withour material" );

	while ( !material->IsLoaded() || !mesh->IsLoaded() || !TexturesLoaded() )
	{
		Sleep( 1 );
	}

	Ecs::Entity Instance = mesh->CreateEntity( World );
	World.Assign<MaterialComponent>( Instance, MaterialComponent{ material } );

	return Instance;
}

void GameObject::WindowResized( int w, int h )
{
	//ASSERT_FAIL( "Not implemented" );
	_Pipeline = nullptr;
	//if ( !pso.IsDirty() )
	//{
	//	pso.SetViewPort( w, h );
	//	pso.Build( material );
	//}
}

bool GameObject::TexturesLoaded() const
{
	for ( int i = 0; i < textures.size(); i++ )
	{
		if ( !textures[ i ]->IsLoaded() )
			return false;
	}

	return true;
}

bool GameObject::IsLoaded() const
{
	return material != nullptr && material->IsLoaded() && mesh->IsLoaded() && TexturesLoaded();
}

void GameObject::Update( )
{
	if ( !IsLoaded() )
		return;

	//if ( mesh->GetMeshType() == MeshType::Skinned )
	//{
	//	std::shared_ptr<SkinnedMesh> pSkinnedMesh = std::static_pointer_cast<SkinnedMesh>( mesh );
	//	pSkinnedMesh->Update( Frame::DeltaTime );
	//}
}

void GameObject::Render( CommandBuffer* pCommandBuffer )
{
	if ( material == nullptr )
		return;

	if ( !material->IsLoaded() || !mesh->IsLoaded() || !TexturesLoaded() )
		return;


	//TODO: Create ubo for material
	if ( mesh->GetMeshType() == MeshType::Skinned && material->GetUniformBuffers().size() == 1)
	{
		std::shared_ptr<SkinnedMesh> pSkinnedMesh = std::static_pointer_cast<SkinnedMesh>( mesh );

		SkinnedMeshBuffer* _ResultBuffer = new SkinnedMeshBuffer{};

		auto localMeshUniformBuffer = new UniformBuffer( { _ResultBuffer, sizeof( SkinnedMeshBuffer ) } );
		material->AddUniformBuffer( localMeshUniformBuffer );
	}

	material->UpdateUniformBuffers();

	//if ( pso .IsDirty() )
	//{
	//	mesh->BuildDescriptors( material );
	//
	//	material->GetSampler()->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, textures[ 0 ]->GetMipLevels() );
	//
	//	pso.Create( material, std::vector<VkDynamicState>(), true );
	//	pso.SetVertexLayout( mesh->GetBindingDescription(), mesh->GetAttributeDescriptions() );
	//	pso.Build( material );
	//}

	if ( _Pipeline == nullptr )
	{
		material->GetSampler()->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, textures[ 0 ]->GetMipLevels() );

		_Pipeline = PipelineStateCache::GetOrCreatePipeline( GraphicsContext::RenderPass, material, {}, EDepthTest::Enabled );
	}

	vkCmdBindPipeline( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, _Pipeline );

	const std::vector<SubMesh*>& meshes = mesh->GetSubMeshes();
	for ( int i = 0; i < meshes.size(); i++ )
	{
		VkDescriptorSet set = material->AccessDescriptorPool().RetrieveDescriptorSet( material, textures[ i ].get(), material->GetSampler().get() );
		vkCmdBindDescriptorSets( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, material->AccessDescriptorPool().GetPipelineLayout(), 0, 1, &set, 0, nullptr );

		meshes[ i ]->Draw( pCommandBuffer );
	}
}
