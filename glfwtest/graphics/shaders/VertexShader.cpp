#include "VertexShader.h"
#include "io\FileSystem.h"

#include <vector>
#include <functional>


VertexShader::VertexShader(const std::string& fileName) :
	Shader()
{
	//Load meta data to create descriptors?
	FileSystem::LoadFileAsync("shaders/" + fileName + ".vert.spv", std::bind(&VertexShader::FileLoaded, this, std::placeholders::_1));

}


VertexShader::~VertexShader()
{

}

void VertexShader::FileLoaded(std::vector<char> fileData)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fileData.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(fileData.data());

	if (vkCreateShaderModule(GraphicsContext::LogicalDevice, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
}