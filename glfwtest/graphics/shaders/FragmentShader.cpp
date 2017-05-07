#include "FragmentShader.h"
#include "io\FileSystem.h"

#include <string>

FragmentShader::FragmentShader(const std::string& fileName, const VkDevice& device) :
	Shader(device)
{
	auto code = FileSystem::ReadFile("shaders/" + fileName + ".frag.spv");

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
	std::memcpy(codeAligned.data(), code.data(), code.size());
	createInfo.pCode = codeAligned.data();

	if (vkCreateShaderModule(device, &createInfo, shaderModule.AllocationCallbacks(), shaderModule.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
}

FragmentShader::~FragmentShader()
{
	
}
