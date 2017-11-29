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

	void SetVertices(const VkVertexInputBindingDescription& input, const std::vector<VkVertexInputAttributeDescription>& descriptions);
	void SetShader(const std::vector<VkPipelineShaderStageCreateInfo>& shaders);

	const InstanceWrapper<VkPipelineLayout>& GetLayout() const;
	const InstanceWrapper<VkPipeline>& GetPipeLine() const;

	void Build();

private:
	InstanceWrapper<VkPipelineLayout> pipelineLayout;
	InstanceWrapper<VkPipeline> graphicsPipeline;

	std::shared_ptr<Material> material;

	bool isDirty;

	VkGraphicsPipelineCreateInfo pipelineInfo;

	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;

};