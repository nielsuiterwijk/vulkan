#pragma once

#include "graphics/GraphicsContext.h"

//TODO: Add ref counting
class Shader
{
protected:
	Shader() :
		shaderModule(GraphicsContext::LogicalDevice, vkDestroyShaderModule, GraphicsContext::GlobalAllocator.Get()),
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
