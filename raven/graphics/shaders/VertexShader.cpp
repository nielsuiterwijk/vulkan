#include "VertexShader.h"
#include "helpers\JsonParsers.h"
#include "io\FileSystem.h"

#include <functional>
#include <vector>

VertexShader::VertexShader( const std::string& fileName ) :
	Shader(),
	filesLeft( 2 )
{
	_ShaderFileName = "shaders/" + fileName;
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
		ASSERT_FAIL( "failed to create shader module!" );
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";

	filesLeft.fetch_sub( 1, std::memory_order_relaxed );
}

void VertexShader::MetaLoaded( std::vector<char> fileData )
{
	_Inputs.clear();

	std::string fileContents( fileData.data(), fileData.size() );

	auto jsonObject = json::parse( fileContents );

	auto inputsJson = jsonObject[ "inputs" ];
	_Inputs.reserve( inputsJson.size() );
	for ( int i = 0; i < inputsJson.size(); i++ )
	{
		auto object = inputsJson[ i ];

		_Inputs.emplace_back( object );
	}

	auto ubos = jsonObject[ "ubos" ];
	_BufferDescriptors.reserve( ubos.size() );
	resourceLayouts.reserve( ubos.size() );
	for ( int i = 0; i < ubos.size(); i++ )
	{
		_BufferDescriptors.emplace_back();
		resourceLayouts.emplace_back();
	}

	for ( int i = 0; i < ubos.size(); i++ )
	{
		auto& object = ubos[ i ];
		/*	"type" : "_43",
			"name" : "Bones",
			"block_size" : 4176,
			"set" : 0,
			"binding" : 1	*/

		int32_t bindingSlot = object[ "binding" ];

		VkDescriptorBufferInfo& bufferInfo = _BufferDescriptors[ bindingSlot ];
		bufferInfo.buffer = nullptr;
		bufferInfo.offset = 0;
		bufferInfo.range = static_cast<VkDeviceSize>( object[ "block_size" ] );

		resourceLayouts[ i ].BindingSlot = bindingSlot;
		resourceLayouts[ i ].Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		resourceLayouts[ i ].ShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
	}

	std::sort( resourceLayouts.begin(), resourceLayouts.end(), []( const ResourceLayout& a, const ResourceLayout& b ) { return a.BindingSlot < b.BindingSlot; } );
	std::sort( std::begin( _Inputs ), std::end( _Inputs ), []( const ShaderInput& a, const ShaderInput& b ) { return a.location < b.location; } );

	_InputSize = 0;
	for ( auto& input : _Inputs )
	{
		input.offset = _InputSize;
		_InputSize += Vulkan::GetSizeFromFormat( input.typeFormat );
	}

	filesLeft.fetch_sub( 1, std::memory_order_relaxed );
}

void VertexShader::GetBindingDescription( VkVertexInputBindingDescription& bindingDescription )
{
	bindingDescription.binding = 0;
	//bindingDescription.stride = sizeof( Vertex );
	bindingDescription.stride = _InputSize;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void VertexShader::GetAttributeDescriptions( std::vector<VkVertexInputAttributeDescription>& attributeDescriptions )
{
	attributeDescriptions.resize( _Inputs.size() );

	for ( int i = 0; i < _Inputs.size(); i++ )
	{
		attributeDescriptions[ i ].binding = 0;
		attributeDescriptions[ i ].location = _Inputs[ i ].location;
		attributeDescriptions[ i ].format = _Inputs[ i ].typeFormat;
		attributeDescriptions[ i ].offset = _Inputs[ i ].offset;
	}
}
