#pragma once

#include "helpers\InstanceWrapper.h"
#include "shaders\Material.h"

class GraphicsDevice;
class Material;

//NU TODO: Every material should have a PipelineStateObject created for it. A model had a material, models with the same material can be rendered ideally in the same draw call if no dynamic data?
class PipelineStateObject 
{
public:
	PipelineStateObject();
	~PipelineStateObject();

	//Invalidates the previous objects
	void Create( const Material* pMaterial, const std::vector<VkDynamicState>& dynamicStates, bool enableDepthTest );
	void Reload();

	void SetViewPort( int width, int height );
	void SetVertexLayout( const VkVertexInputBindingDescription& input, const std::vector<VkVertexInputAttributeDescription>& descriptions );
	void SetShader( const std::vector<VkPipelineShaderStageCreateInfo>& shaders );

	//const InstanceWrapper<VkPipeline>& GetPipeLine() const;

	VkPipeline Build( const Material* pMaterial );

	bool IsDirty() const { return isDirty; }

private:
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
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineDynamicStateCreateInfo dynamicState;

	std::vector<VkDynamicState> _ActiveDynamicStates;
};
