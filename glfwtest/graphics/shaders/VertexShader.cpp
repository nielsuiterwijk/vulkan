#include "VertexShader.h"
#include "io\FileSystem.h"

#include <vector>


VertexShader::VertexShader(const std::string& fileName) :
	Shader()
{
	auto code = FileSystem::ReadFile("shaders/" + fileName + ".vert.spv");

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
	std::memcpy(codeAligned.data(), code.data(), code.size());
	createInfo.pCode = codeAligned.data();

	if (vkCreateShaderModule(GraphicsContext::LogicalDevice, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";

}


VertexShader::~VertexShader()
{

}