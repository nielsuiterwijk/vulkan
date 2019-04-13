#include "FragmentShader.h"
#include "io\FileSystem.h"

#include <functional>
#include <string>

FragmentShader::FragmentShader( const std::string& fileName ) :
	Shader()
{
	FileSystem::LoadFileAsync( "shaders/" + fileName + ".frag.spv", std::bind( &FragmentShader::FileLoaded, this, std::placeholders::_1 ) );
}

FragmentShader::~FragmentShader()
{
}

void FragmentShader::FileLoaded( std::vector<char> fileData )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fileData.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>( fileData.data() );

	if ( vkCreateShaderModule( GraphicsContext::LogicalDevice, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace() ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create shader module!" );
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";

	isLoaded = true;

	//materialQueueMutex.lock();
	//while (!materialQueue.empty())
	//{
	//	AddToShaderStage(materialQueue.front());
	//	materialQueue.pop();
	//}
	//materialQueueMutex.unlock();
}
