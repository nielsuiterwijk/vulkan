#pragma once

#include "../helpers/InstanceWrapper.h"

//TODO: Add ref counting
class Shader
{
protected:
	Shader(const VkDevice& device) :
		shaderModule(device, vkDestroyShaderModule)
	{
	}

	virtual ~Shader() 
	{
		shaderModule = nullptr;
	}

protected:

	VkPipelineShaderStageCreateInfo shaderInfo;
	InstanceWrapper<VkShaderModule> shaderModule;
};
