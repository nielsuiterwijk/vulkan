#include "FragmentShader.h"

#include <functional>
#include <string>

#include "io\FileSystem.h"

FragmentShader::FragmentShader( const std::string& fileName )
	: Shader()
	, _FilesLeft( 2 )
{
	auto ShaderLoadedLambda = [&]( std::vector<char> fileData ) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileData.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>( fileData.data() );

		if ( vkCreateShaderModule( GraphicsContext::LogicalDevice, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace() ) != VK_SUCCESS )
		{
			ASSERT_FAIL( "failed to create shader module!" );
		}

		shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderInfo.module = shaderModule;
		shaderInfo.pName = "main";

		_FilesLeft.fetch_sub( 1, std::memory_order_relaxed );
	};


	_ShaderFileName = "shaders/" + fileName;
	FileSystem::LoadFileAsync( "shaders/" + fileName + ".frag.spv", ShaderLoadedLambda ); // std::bind(&FragmentShader::ShaderLoaded, this, std::placeholders::_1) );
	FileSystem::LoadFileAsync( "shaders/" + fileName + ".frag.json", std::bind( &FragmentShader::MetaLoaded, this, std::placeholders::_1 ) );
}

FragmentShader::~FragmentShader()
{
}

void FragmentShader::ShaderLoaded( std::vector<char> fileData )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fileData.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>( fileData.data() );

	if ( vkCreateShaderModule( GraphicsContext::LogicalDevice, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace() ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to create shader module!" );
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";

	_FilesLeft.fetch_sub( 1, std::memory_order_relaxed );

	//materialQueueMutex.lock();
	//while (!materialQueue.empty())
	//{
	//	AddToShaderStage(materialQueue.front());
	//	materialQueue.pop();
	//}
	//materialQueueMutex.unlock();
}

void FragmentShader::MetaLoaded( std::vector<char> fileData )
{
	std::string fileContents( fileData.data(), fileData.size() );

	auto jsonObject = json::parse( fileContents );

	auto textures = jsonObject[ "textures" ];
	resourceLayouts.reserve( textures.size() );
	for ( int i = 0; i < textures.size(); i++ )
	{
		resourceLayouts.emplace_back();
	}

	for ( int i = 0; i < textures.size(); i++ )
	{
		auto object = textures[ i ];
		/*	"type" : "_43",
			"name" : "Bones",
			"block_size" : 4176,
			"set" : 0,
			"binding" : 1	*/

		int32_t bindingSlot = object[ "binding" ];

		resourceLayouts[ i ].BindingSlot = bindingSlot;
		resourceLayouts[ i ].Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		resourceLayouts[ i ].ShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	_FilesLeft.fetch_sub( 1, std::memory_order_relaxed );
}
