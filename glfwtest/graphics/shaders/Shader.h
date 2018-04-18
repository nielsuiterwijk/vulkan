#pragma once

#include "graphics/GraphicsContext.h"

#include <queue>

//TODO: Add ref counting
class Shader
{
protected:
	Shader() :
		shaderModule(GraphicsContext::LogicalDevice, vkDestroyShaderModule, GraphicsContext::GlobalAllocator.Get()),
		shaderInfo(),
		isLoaded(false)
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

	bool IsLoaded() const { return isLoaded; }

	void AddToShaderStage(std::vector<VkPipelineShaderStageCreateInfo>* stages)
	{
		if (IsLoaded())
		{
			stages->push_back(shaderInfo);
		}
		else
		{
			materialQueueMutex.lock();
			materialQueue.push(stages);
			materialQueueMutex.unlock();
		}
	}

protected:

	VkPipelineShaderStageCreateInfo shaderInfo;
	InstanceWrapper<VkShaderModule> shaderModule;

	bool isLoaded;

	std::mutex materialQueueMutex;
	std::queue< std::vector<VkPipelineShaderStageCreateInfo>* > materialQueue;

};
