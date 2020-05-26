#include "Material.h"
#include "FragmentShader.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "graphics/buffers/UniformBuffer.h"
#include "io\FileSystem.h"

#include "graphics/textures/TextureLoader.h"
#include "graphics/textures/TextureSamplerCache.h"

#include "helpers/Murmur3.h"

#include <iostream>
#include <thread>
#include <windows.h>

Material::Material( const std::string& fileName ) :
	vertex( nullptr ),
	fragment( nullptr )
{
	std::cout << "Creating material: " << fileName << std::endl;
	FileSystem::LoadFileAsync( "materials/" + fileName + ".mat", std::bind( &Material::FileLoaded, this, std::placeholders::_1 ) );
}

Material::~Material()
{
	vertex = nullptr;
	fragment = nullptr;

	std::cout << "Destroyed material" << std::endl;
}

uint32_t Material::CalcHash() const
{
	ASSERT( IsLoaded() );
	uint32_t Hash = Murmur3::Hash( vertex->GetFileName() );
	Hash = Murmur3::Hash( fragment->GetFileName(), Hash );

	return Hash;
}

#include "graphics/PipelineStateCache.h"

//TODO: It would be really cool if there was a way for an object (gameobject, material) to be saved.
void Material::FileLoaded( std::vector<char> fileData )
{
	std::string fileContents( fileData.data(), fileData.size() );
	auto jsonObject = json::parse( fileContents );

	vertex = ShaderCache::GetVertexShader( jsonObject[ "shader" ] );
	fragment = ShaderCache::GetFragmentShader( jsonObject[ "shader" ] );

	if ( jsonObject.find( "texture" ) != jsonObject.end() )
	{
		//texture = TextureLoader::Get( jsonObject[ "texture" ] );
		ASSERT_FAIL( "Unsupported, textures should be defined with the game object" );
	}

	//sampler = std::make_shared<TextureSampler>();
	//sampler->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 );
	_TextureSamplerHash = TextureSamplerCache::CalcHash( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 );
	TextureSamplerCache::GetOrCreateSampler( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 );
	//NU TODO: add sampler options in material

	while ( !vertex->IsLoaded() || !fragment->IsLoaded() )
	{
		Sleep( 1 );
	}

	std::cout << "Initializing shader stages" << std::endl;
	shaderStages.push_back( vertex->GetShaderStageCreateInfo() );
	shaderStages.push_back( fragment->GetShaderStageCreateInfo() );

	descriptorPool.SetupBindings( vertex, fragment );

	//NU TODO: add pipeline options in material
	PipelineBuilder Builder( GraphicsContext::RenderPass );
	Builder.SetDepthTest( true );
	Builder.SetDepthWrite( true );
	
	//TODO: Maybe it should only store the Pipeline builder instead of the hash.
	_PipelineHash = PipelineStateCache::CreatePipeline( Builder, this, {} );
	ASSERT( _PipelineHash != 0 );
}

const std::vector<VkPipelineShaderStageCreateInfo>& Material::GetShaderStages() const
{
	return shaderStages;
}

bool Material::IsLoaded() const
{
	if ( vertex == nullptr || fragment == nullptr )
	{
		return false;
	}

	return vertex->IsLoaded() && fragment->IsLoaded();
}
