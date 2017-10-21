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


	const InstanceWrapper<VkPipelineLayout>& GetLayout() const;
	const InstanceWrapper<VkPipeline>& GetPipeLine() const;

private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout;
	InstanceWrapper<VkPipeline> graphicsPipeline;

};