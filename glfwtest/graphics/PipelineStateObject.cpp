#include "PipelineStateObject.h"

#include "GraphicsDevice.h"

#include "RenderPass.h"

PipelineStateObject::PipelineStateObject() :
	pipelineLayout(),
	graphicsPipeline(),
	material(nullptr)
{
}

PipelineStateObject::PipelineStateObject(std::shared_ptr<Material> material) :
	pipelineLayout(GraphicsContext::LogicalDevice, vkDestroyPipelineLayout, GraphicsContext::GlobalAllocator.Get()),
	graphicsPipeline(GraphicsContext::LogicalDevice, vkDestroyPipeline, GraphicsContext::GlobalAllocator.Get()),
	material(material)
{
	Create(material);

	GraphicsDevice::Instance().OnSwapchainInvalidated(std::bind(&PipelineStateObject::Reload, this));
}

PipelineStateObject::~PipelineStateObject()
{
	pipelineLayout = nullptr;
	graphicsPipeline = nullptr;
	material = nullptr;
	std::cout << "Destroyed PSO" << std::endl;
}

void PipelineStateObject::Reload()
{
	Create(material);
}

void PipelineStateObject::Create(std::shared_ptr<Material> material)
{
	this->material = material;
	pipelineLayout = nullptr;
	graphicsPipeline = nullptr;

	pipelineLayout = InstanceWrapper<VkPipelineLayout>(GraphicsContext::LogicalDevice, vkDestroyPipelineLayout, GraphicsContext::GlobalAllocator.Get());
	graphicsPipeline = InstanceWrapper<VkPipeline>(GraphicsContext::LogicalDevice, vkDestroyPipeline, GraphicsContext::GlobalAllocator.Get());

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = GraphicsContext::WindowSize.x; 
	viewport.height = GraphicsContext::WindowSize.y; //TODO: Need to be taken as input?
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = viewport.width;
	scissor.extent.height = viewport.height;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //Setting this to VK_TRUE requires enabling a GPU feature.
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //If set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Wireframe mode: VK_POLYGON_MODE_LINE or point cloud: VK_POLYGON_MODE_POINT
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


											//TODO: Look into MSAA
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; /// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional


											   //Per frame buffer, default alpha blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	//Global state
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

	if (vkCreatePipelineLayout(GraphicsContext::LogicalDevice, &pipelineLayoutInfo, pipelineLayout.AllocationCallbacks(), pipelineLayout.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = material->GetShaderStages();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(GraphicsContext::LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, graphicsPipeline.AllocationCallbacks(), graphicsPipeline.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}


const InstanceWrapper<VkPipelineLayout>& PipelineStateObject::GetLayout() const
{
	return pipelineLayout;
}

const InstanceWrapper<VkPipeline>& PipelineStateObject::GetPipeLine() const
{
	return graphicsPipeline;
}