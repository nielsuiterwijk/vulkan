#pragma once

#include "../helpers/InstanceWrapper.h"
#include "graphics\GraphicsDevice.h"

//TODO: Add ref counting
class Shader
{
protected:
	Shader() :
		shaderModule(GraphicsContext::LogicalDevice, vkDestroyShaderModule)
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
