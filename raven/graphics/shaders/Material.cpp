#include "Material.h"
#include "FragmentShader.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "graphics/buffers/UniformBuffer.h"
#include "io\FileSystem.h"

#include "graphics/textures/TextureLoader.h"
#include "graphics/textures/TextureSampler.h"

#include <iostream>
#include <thread>
#include <windows.h>

Material::Material( const std::string& fileName ) :
	vertex( nullptr ),
	fragment( nullptr ),
	texture( nullptr ),
	sampler( nullptr )
{
	std::cout << "Creating material: " << fileName << std::endl;
	FileSystem::LoadFileAsync( "materials/" + fileName + ".mat", std::bind( &Material::FileLoaded, this, std::placeholders::_1 ) );
}

Material::~Material()
{
	vertex = nullptr;
	fragment = nullptr;

	for ( int i = 0; i < uniformBuffers.size(); i++ )
	{
		delete uniformBuffers[ i ];
	}
	uniformBuffers.clear();

	texture = nullptr;
	sampler = nullptr;

	std::cout << "Destroyed material" << std::endl;
}

void Material::FileLoaded( std::vector<char> fileData )
{
	std::string fileContents( fileData.data(), fileData.size() );
	auto jsonObject = json::parse( fileContents );

	vertex = ShaderCache::GetVertexShader( jsonObject[ "shader" ] );
	fragment = ShaderCache::GetFragmentShader( jsonObject[ "shader" ] );

	if ( jsonObject.find( "texture" ) != jsonObject.end() )
	{
		texture = TextureLoader::Get( jsonObject[ "texture" ] );
	}

	sampler = std::make_shared<TextureSampler>();
	sampler->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 );
}

void Material::AddUniformBuffer( UniformBuffer* pUniformBuffer )
{
	assert( pUniformBuffer != nullptr );
	uniformBuffers.push_back( pUniformBuffer );
}

void Material::UpdateUniformBuffers()
{
	for ( UniformBuffer* buffer : uniformBuffers )
	{
		buffer->Upload();
	}
}

const std::vector<VkPipelineShaderStageCreateInfo>& Material::GetShaderStages()
{
	//TODO: Not the right place, should be done on a callback of some sorts.
	if ( IsLoaded() && shaderStages.size() == 0 )
	{
		std::cout << "Initializing shader stages" << std::endl;
		shaderStages.push_back( vertex->GetShaderStageCreateInfo() );
		shaderStages.push_back( fragment->GetShaderStageCreateInfo() );

		descriptorPool.SetupBindings( vertex, fragment );
	}

	return shaderStages;
}

bool Material::IsLoaded() const
{
	if ( vertex == nullptr || fragment == nullptr )
	{
		return false;
	}

	return vertex->IsLoaded() && fragment->IsLoaded() && ( texture == nullptr || texture->IsLoaded() );
}
