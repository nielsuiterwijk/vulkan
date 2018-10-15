#include "PipelineStateObject.h"

#include "GraphicsDevice.h"
#include "RenderPass.h"
#include "graphics/buffers/UniformBuffer.h"



PipelineStateObject::PipelineStateObject() :
	graphicsPipeline(),
	material(nullptr),
	isDirty(true),
	vertexInputInfo(),
	colorBlending(),
	colorBlendAttachment(),
	multisampling(),
	rasterizer(),
	viewportState(),
	inputAssembly(),
	depthStencil()
{
}

PipelineStateObject::PipelineStateObject(std::shared_ptr<Material> material) :
	graphicsPipeline(GraphicsContext::LogicalDevice, vkDestroyPipeline, GraphicsContext::GlobalAllocator.Get()),
	material(material),
	isDirty(true),
	vertexInputInfo(),
	colorBlending(),
	colorBlendAttachment(),
	multisampling(),
	rasterizer(),
	viewportState(),
	inputAssembly(),
	depthStencil()
{
	Create(material, std::vector<VkDynamicState>(), true);

	GraphicsDevice::Instance().OnSwapchainInvalidated(std::bind(&PipelineStateObject::Reload, this));
}

PipelineStateObject::~PipelineStateObject()
{
	graphicsPipeline = nullptr;
	material = nullptr;
	std::cout << "Destroyed PSO" << std::endl;
}

void PipelineStateObject::Reload()
{
	Create(material, std::vector<VkDynamicState>(), true);
}

/*void CreatePSO( VertexBuffer& vertexBuffer, 
				  Shader& shader,
				  GfxDevice::BlendMode blendMode,
				  GfxDevice::DepthFunc depthFunc,
				  GfxDevice::CullMode cullMode,
				  GfxDevice::FillMode fillMode,
				  VkRenderPass renderPass, 
				  GfxDevice::PrimitiveTopology topology, 
				  std::uint64_t hash )
*/
void PipelineStateObject::Create(std::shared_ptr<Material> material, const std::vector<VkDynamicState>& dynamicStates, bool enableDepthTest)
{
	this->material = material;
	graphicsPipeline = nullptr;

	graphicsPipeline = InstanceWrapper<VkPipeline>(GraphicsContext::LogicalDevice, vkDestroyPipeline, GraphicsContext::GlobalAllocator.Get());


	inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = GraphicsContext::WindowSize.x; 
	viewport.height = GraphicsContext::WindowSize.y; //TODO: Need to be taken as input?
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = viewport.width;
	scissor.extent.height = viewport.height;

	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	activeDynamicStates.resize(dynamicStates.size());
	for (size_t i = 0; i < dynamicStates.size(); i++)
	{
		activeDynamicStates[i] = dynamicStates[i];
	}
	
	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = activeDynamicStates.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(activeDynamicStates.size());

	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //Setting this to VK_TRUE requires enabling a GPU feature.
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //If set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Wireframe mode: VK_POLYGON_MODE_LINE or point cloud: VK_POLYGON_MODE_POINT
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


	//TODO: Look into MSAA
	multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; /// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional


	 //Per frame buffer, default alpha blending
	colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	
	//Global state
	colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	vertexInputInfo.flags = 0;	//reserved for future use.
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	if (enableDepthTest)
	{
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		//Stencil operations
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
	}
	
	pipelineInfo = {};
	if (material != nullptr)
	{
		SetShader(material->GetShaderStages());
	}
	isDirty = true;
}

void PipelineStateObject::SetViewPort(int width, int height)
{
	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = viewport.width;
	scissor.extent.height = viewport.height;

	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	isDirty = true;
}

void PipelineStateObject::SetVertexLayout(const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
{
	vertexInputInfo.flags = 0;	//reserved for future use.
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	isDirty = true;
	//Build();
}

void PipelineStateObject::SetShader(const std::vector<VkPipelineShaderStageCreateInfo>& shaders)
{
	pipelineInfo.stageCount = static_cast<uint32_t>(shaders.size());
	pipelineInfo.pStages = shaders.data();
	isDirty = true;
}

const InstanceWrapper<VkPipeline>& PipelineStateObject::GetPipeLine() const
{
	assert(!isDirty);
	return graphicsPipeline;
}

void PipelineStateObject::Build()
{
	assert(isDirty);
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	if (dynamicState.dynamicStateCount == 0)
	{
		pipelineInfo.pDynamicState = nullptr;
	}
	else
	{
		pipelineInfo.pDynamicState = &dynamicState;
	}
	pipelineInfo.layout = GraphicsContext::PipelineLayout;
	pipelineInfo.renderPass = GraphicsContext::RenderPass->GetNative();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(GraphicsContext::LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, graphicsPipeline.AllocationCallbacks(), graphicsPipeline.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	isDirty = false;
}