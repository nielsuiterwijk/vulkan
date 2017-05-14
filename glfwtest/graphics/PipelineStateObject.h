#pragma once

#include "helpers\InstanceWrapper.h"
#include "shaders\Material.h"

#include "RenderPass.h"

class GraphicsDevice;

class PipelineStateObject
{
public:
	PipelineStateObject(std::shared_ptr<Material> material, std::shared_ptr<RenderPass> renderpass);
	~PipelineStateObject();

private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout;
	InstanceWrapper<VkPipeline> graphicsPipeline;

};