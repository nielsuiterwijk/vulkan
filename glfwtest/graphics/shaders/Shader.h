#pragma once

#include "../helpers/InstanceWrapper.h"
#include "graphics\GraphicsDevice.h"

//TODO: Add ref counting
class Shader
{
protected:
	Shader() :
		shaderModule(GraphicsContext::LogicalDevice, vkDestroyShaderModule),
		shaderInfo()
	{
	}

	virtual ~Shader()
	{
		shaderModule = nullptr;
	}
public:

	const VkPipelineShaderStageCreateInfo& GetShaderStageCreateInfo() const
	{
		return shaderInfo;
	}

protected:

	VkPipelineShaderStageCreateInfo shaderInfo;
	InstanceWrapper<VkShaderModule> shaderModule;

};
