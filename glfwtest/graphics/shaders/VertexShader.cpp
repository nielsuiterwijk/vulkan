#include "VertexShader.h"
#include "helpers\JsonParsers.h"
#include "io\FileSystem.h"

#include <functional>
#include <vector>

VertexShader::VertexShader( const std::string& fileName ) :
	Shader(),
	filesLeft( 2 )
{
#if DEBUG
	shaderFileName = "shaders/" + fileName;
#endif
	FileSystem::LoadFileAsync( "shaders/" + fileName + ".vert.spv", std::bind( &VertexShader::ShaderLoaded, this, std::placeholders::_1 ) );
	FileSystem::LoadFileAsync( "shaders/" + fileName + ".vert.json", std::bind( &VertexShader::MetaLoaded, this, std::placeholders::_1 ) );
}

VertexShader::~VertexShader()
{
}

void VertexShader::ShaderLoaded( std::vector<char> fileData )
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
	shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";

	filesLeft.fetch_sub( 1, std::memory_order_relaxed );
}

void VertexShader::MetaLoaded( std::vector<char> fileData )
{
	inputs.clear();

	std::string fileContents( fileData.data(), fileData.size() );

	auto jsonObject = json::parse( fileContents );

	auto inputsJson = jsonObject[ "inputs" ];
	inputs.reserve( inputsJson.size() );
	for ( int i = 0; i < inputsJson.size(); i++ )
	{
		auto object = inputsJson[ i ];

		inputs.emplace_back( object );
	}

	auto ubos = jsonObject[ "ubos" ];
	bufferDescriptors.reserve( ubos.size() );
	for ( int i = 0; i < ubos.size(); i++ )
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferDescriptors.emplace_back( bufferInfo );
	}

	for ( int i = 0; i < ubos.size(); i++ )
	{
		auto object = ubos[ i ];
		/*	"type" : "_43",
			"name" : "Bones",
			"block_size" : 4176,
			"set" : 0,
			"binding" : 1	*/

		VkDescriptorBufferInfo& bufferInfo = bufferDescriptors[ object[ "binding" ] ];
		bufferInfo.buffer = nullptr;
		bufferInfo.offset = 0;
		bufferInfo.range = static_cast<VkDeviceSize>( object[ "block_size" ] );
	}

	std::sort( std::begin( inputs ), std::end( inputs ), []( const ShaderInput& a, const ShaderInput& b ) { return a.location < b.location; } );

	filesLeft.fetch_sub( 1, std::memory_order_relaxed );
}

void VertexShader::GetBindingDescription( VkVertexInputBindingDescription& bindingDescription )
{
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof( Vertex );
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void VertexShader::GetAttributeDescriptions( std::vector<VkVertexInputAttributeDescription>& attributeDescriptions )
{
	attributeDescriptions.resize( inputs.size() );

	for ( int i = 0; i < inputs.size(); i++ )
	{
		attributeDescriptions[ i ].binding = 0;
		attributeDescriptions[ i ].location = inputs[ i ].location;
		attributeDescriptions[ i ].format = inputs[ i ].typeFormat;
		attributeDescriptions[ i ].offset = inputs[ i ].offset;
	}
}
