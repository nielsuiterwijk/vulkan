#pragma once

#include "helpers\InstanceWrapper.h"
#include "shaders\Material.h"

#include "RenderPass.h"

class GraphicsDevice;

class PipelineStateObject
{
public:
	PipelineStateObject(std::shared_ptr<Material> material);
	~PipelineStateObject();

private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout;
	InstanceWrapper<VkPipeline> graphicsPipeline;

};