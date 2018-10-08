#pragma once

#include "graphics/GraphicsContext.h"

#include <queue>

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

	virtual bool IsLoaded() const = 0;
	
protected:
	VkPipelineShaderStageCreateInfo shaderInfo;
	InstanceWrapper<VkShaderModule> shaderModule;

};
