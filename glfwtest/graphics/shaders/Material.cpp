#include "Material.h"

#include "io\FileSystem.h"

Material::Material(const std::string& fileName, InstanceWrapper<VkDevice>& device) :
	vertexShaderModule(device, vkDestroyShaderModule),
	fragmentShaderModule(device, vkDestroyShaderModule)
{
	auto vertShaderCode = FileSystem::ReadFile("shaders/" + fileName + ".vert.spv");
	auto fragShaderCode = FileSystem::ReadFile("shaders/" + fileName + ".frag.spv");

	BuildShaderModule(vertShaderCode, vertexShaderModule, device);
	BuildShaderModule(fragShaderCode, fragmentShaderModule, device);

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	//Needed at some point.. not sure who and where :D
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };
}

void Material::BuildShaderModule(const std::vector<char>& code, InstanceWrapper<VkShaderModule>& shaderModule, InstanceWrapper<VkDevice>& device)
{
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
}

Material::~Material()
{
	vertexShaderModule = nullptr;
	fragmentShaderModule = nullptr;
}
