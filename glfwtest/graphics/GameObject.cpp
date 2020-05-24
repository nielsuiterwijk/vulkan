#include "GameObject.h"

#include "graphics/PipelineStateCache.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/models/MeshFileLoader.h"
#include "graphics/models/SkinnedMesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureLoader.h"
#include "io/FileSystem.h"

GameObject::GameObject( const std::string& objectFile )
	: material( nullptr )
	, mesh( nullptr )
{
	FileSystem::LoadFileAsync( "gameobjects/" + objectFile + ".gameobject", std::bind( &GameObject::FileLoaded, this, std::placeholders::_1 ) );
}

GameObject::~GameObject()
{
	material = nullptr;
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
				_Textures.push_back( _Textures[ j ] );
				alreadyAdded = true;
				break;
			}
		}

		if ( !alreadyAdded )
		{
			textureFileNames.push_back( textureFileName );
			_Textures.push_back( TextureLoader::Get( textureFileName ) );
		}
	}
}

#include "ecs/World.h"

Ecs::Entity GameObject::CreateInstance( Ecs::World& World, int32_t Count )
{
	ASSERT( Count == 1 );
	//ASSERT( material->GetUniformBuffers().size() == 0 );

	if ( material == nullptr )
		ASSERT_FAIL( "GameObject withour material" );
	if ( mesh == nullptr )
		ASSERT_FAIL( "GameObject withour material" );

	while ( !material->IsLoaded() || !mesh->IsLoaded() || !TexturesLoaded() )
	{
		Sleep( 1 );
	}

	Ecs::Entity Instance = World.Create();
	mesh->Assign( World, Instance );
	World.Assign<MaterialComponent>( Instance, MaterialComponent { material } );
	World.Assign<Texture2DComponent>( Instance, Texture2DComponent { _Textures } );

	//TODO: It does not scale well if the material owns the UBO's. This should probably it's own little component.
	if ( mesh->GetMeshType() == MeshType::Skinned && material->GetUniformBuffers().size() == 1 )
	{				
		//TODO: Get `SkinnedMeshComponent` assigned to this entity. Link the buffer of that component to the material.
		SkinnedMeshBuffer* _ResultBuffer = new SkinnedMeshBuffer {};

		auto localMeshUniformBuffer = new UniformBuffer( { _ResultBuffer, sizeof( SkinnedMeshBuffer ) } );
		material->AddUniformBuffer( localMeshUniformBuffer );
	}

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
	for ( int i = 0; i < _Textures.size(); i++ )
	{
		if ( !_Textures[ i ]->IsLoaded() )
			return false;
	}

	return true;
}

bool GameObject::IsLoaded() const
{
	return material != nullptr && material->IsLoaded() && mesh->IsLoaded() && TexturesLoaded();
}

void GameObject::Update()
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

	//const Material* pMaterial = material.get();

	//if ( mesh->GetMeshType() == MeshType::Skinned && material->GetUniformBuffers().size() == 1 )
	//{
	//	std::shared_ptr<SkinnedMesh> pSkinnedMesh = std::static_pointer_cast<SkinnedMesh>( mesh );
	//
	//	SkinnedMeshBuffer* _ResultBuffer = new SkinnedMeshBuffer {};
	//
	//	auto localMeshUniformBuffer = new UniformBuffer( { _ResultBuffer, sizeof( SkinnedMeshBuffer ) } );
	//	material->AddUniformBuffer( localMeshUniformBuffer );
	//}

	//material->UpdateUniformBuffers();

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

	//if ( _Pipeline == nullptr )
	//{
	//	//material->GetSampler()->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, _Textures[ 0 ]->GetMipLevels() );
	//
	//
	//	PipelineBuilder Builder( GraphicsContext::RenderPass );
	//	Builder.SetDepthTest( true );
	//	Builder.SetDepthWrite( true );
	//
	//	_Pipeline = PipelineStateCache::GetOrCreatePipeline( Builder, material.get(), {} );
	//}
	//
	//vkCmdBindPipeline( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, _Pipeline );
	//
	//const std::vector<SubMesh*>& meshes = mesh->GetSubMeshes();
	//for ( int i = 0; i < meshes.size(); i++ )
	//{
	//	VkDescriptorSet set = material->AccessDescriptorPool().RetrieveDescriptorSet( pMaterial, _Textures[ i ].get() );
	//	vkCmdBindDescriptorSets( pCommandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, material->AccessDescriptorPool().GetPipelineLayout(), 0, 1, &set, 0, nullptr );
	//
	//	meshes[ i ]->Draw( pCommandBuffer );
	//}
}
