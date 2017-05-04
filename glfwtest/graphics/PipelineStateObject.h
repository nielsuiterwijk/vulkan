#pragma once

#include "helpers\InstanceWrapper.h"
#include "shaders\Material.h"

class GraphicsDevice;

class PipelineStateObject
{
public:
	PipelineStateObject(GraphicsDevice* graphicsDevice, const Material& material);
	~PipelineStateObject();
	
private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout; 
	InstanceWrapper<VkPipeline> graphicsPipeline;

};