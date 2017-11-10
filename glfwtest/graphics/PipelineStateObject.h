#pragma once

#include "helpers\InstanceWrapper.h"
#include "shaders\Material.h"


class GraphicsDevice;

class PipelineStateObject
{
public:
	PipelineStateObject();
	PipelineStateObject(std::shared_ptr<Material> material);
	~PipelineStateObject();

	//Invalidates the previous objects
	void Create(std::shared_ptr<Material> material);
	void Reload();

	const InstanceWrapper<VkPipelineLayout>& GetLayout() const;
	const InstanceWrapper<VkPipeline>& GetPipeLine() const;

private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout;
	InstanceWrapper<VkPipeline> graphicsPipeline;

	std::shared_ptr<Material> material;

};