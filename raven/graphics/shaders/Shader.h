#pragma once

#include "graphics/GraphicsContext.h"

#include <queue>

struct ResourceLayout
{
	VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	VkShaderStageFlags ShaderStage = VK_SHADER_STAGE_ALL;
	uint32_t BindingSlot = -1;
	uint32_t ArraySize = 1;
};

//TODO: Add ref counting
class Shader
{
protected:
	Shader() :
		shaderModule( GraphicsContext::LogicalDevice, vkDestroyShaderModule, GraphicsContext::GlobalAllocator.Get() ),
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

	const std::vector<ResourceLayout>& GetResourceLayout() { return resourceLayouts; }

	const std::string& GetFileName() const { return _ShaderFileName; }

protected:
	VkPipelineShaderStageCreateInfo shaderInfo = {};
	InstanceWrapper<VkShaderModule> shaderModule;

	std::vector<ResourceLayout> resourceLayouts;

	std::string _ShaderFileName;
};
