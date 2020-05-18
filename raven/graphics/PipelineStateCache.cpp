#include "PipelineStateCache.h"

#include "graphics/PipelineStateObject.h"
#include "graphics/RenderPass.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "helpers/Murmur3.h"


static std::unordered_map<uint32_t, VkPipeline> Cache;

PipelineBuilder::PipelineBuilder()
{
	inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>( GraphicsContext::WindowSize.x );
	viewport.height = static_cast<float>( GraphicsContext::WindowSize.y ); //TODO: Need to be taken as input?
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = static_cast<uint32_t>( viewport.width );
	scissor.extent.height = static_cast<uint32_t>( viewport.height );

	/*viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;*/

	/*dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = nullptr;
	dynamicState.dynamicStateCount = 0;*/

	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //Setting this to VK_TRUE requires enabling a GPU feature.
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //If set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Wireframe mode: VK_POLYGON_MODE_LINE or point cloud: VK_POLYGON_MODE_POINT
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
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
	/*colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional*/

	vertexInputInfo.flags = 0; //reserved for future use.
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
}


void PipelineBuilder::SetViewport( uint32_t Width, uint32_t Height )
{
	viewport.width = static_cast<float>( Width );
	viewport.height = static_cast<float>( Height );
}

void PipelineBuilder::SetScissor( uint32_t Width, uint32_t Height )
{

	scissor.extent.width = static_cast<uint32_t>( Width );
	scissor.extent.height = static_cast<uint32_t>( Height );
}
void PipelineBuilder::SetViewportAndScissor( uint32_t Width, uint32_t Height )
{
	SetViewport( Width, Height );
	SetScissor( Width, Height );
}

void PipelineBuilder::SetColorBlendMode( VkBlendFactor Src, VkBlendFactor Dst )
{
	colorBlendAttachment.srcColorBlendFactor = Src;
	colorBlendAttachment.dstColorBlendFactor = Dst;
}
void PipelineBuilder::SetAlphaBlendMode( VkBlendFactor Src, VkBlendFactor Dst )
{
	colorBlendAttachment.srcAlphaBlendFactor = Src;
	colorBlendAttachment.dstAlphaBlendFactor = Dst;
}

void PipelineBuilder::SetCullMode( VkCullModeFlagBits Bits )
{
	rasterizer.cullMode = Bits;
}

void PipelineBuilder::SetFrontFace( VkFrontFace Face )
{
	rasterizer.frontFace = Face;
}

void PipelineBuilder::SetPolygonMode( VkPolygonMode Mode )
{
	rasterizer.polygonMode = Mode;
}

void PipelineBuilder::SetPrimitive( VkPrimitiveTopology Topology )
{
	inputAssembly.topology = Topology;
}

void PipelineStateCache::Destroy()
{
	//ffs
	//Cache.clear();

	for ( auto& it : Cache )
	{
		vkDestroyPipeline( GraphicsContext::LogicalDevice, it.second, GraphicsContext::GlobalAllocator.Get() );
	}
	Cache.clear();
}

VkPipeline PipelineStateCache::BuildPipeline( const PipelineBuilder& Builder, const std::vector<VkDynamicState>& DynamicStates, EDepthTest DepthTest )
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	VkPipelineViewportStateCreateInfo viewportState = {};
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	VkPipelineDynamicStateCreateInfo dynamicState = {};

	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &Builder.viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &Builder.scissor;

	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = DynamicStates.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>( DynamicStates.size() );

	//Global state
	colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &Builder.colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	if ( DepthTest != EDepthTest::Disabled )
	{
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		//Stencil operations
		if ( DepthTest == EDepthTest::EnabledWithStencil )
		{
			ASSERT_FAIL( "Unsupported" );
		}
		else
		{
			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {};
			depthStencil.back = {};
		}
	}

	{
		VkGraphicsPipelineCreateInfo pipelineInfo = {};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &Builder.inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &Builder.rasterizer;
		pipelineInfo.pMultisampleState = &Builder.multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = dynamicState.dynamicStateCount == 0 ? nullptr : &dynamicState;
		//pipelineInfo.layout = pMaterial->GetDescriptorPool().GetPipelineLayout();
		pipelineInfo.renderPass = GraphicsContext::RenderPass->GetNative();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		VkPipeline Result;

		if ( vkCreateGraphicsPipelines( GraphicsContext::LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, GraphicsContext::GlobalAllocator.Get(), &Result ) != VK_SUCCESS )
		{
			ASSERT_FAIL( "failed to create graphics pipeline!" );
		}


		return Result;
	}
}

VkPipeline PipelineStateCache::GetOrCreatePipeline( const PipelineBuilder& Builder, std::shared_ptr<Material> Material )
{
	return nullptr;
}

VkPipeline
PipelineStateCache::GetOrCreatePipeline( std::shared_ptr<RenderPass> RenderPass, std::shared_ptr<Material> Material, const std::vector<VkDynamicState>& DynamicStates, EDepthTest DepthTest, glm::ivec4 ViewPort )
{
	//NU TODO: Lock this for thread safety?
	uint32_t Hash = Murmur3::Hash( RenderPass->GetHash() );
	Hash = Murmur3::Hash( Material->Hash(), Hash );
	Hash = Murmur3::Hash( (uint32_t)DepthTest, Hash );
	Hash = Murmur3::Hash( &ViewPort, sizeof(glm::ivec4), Hash );

	auto It = Cache.find( Hash );
	if ( It != Cache.end() )
	{
		return It->second;
	}

	PipelineStateObject Pso;
	Pso.Create( Material, DynamicStates, DepthTest == EDepthTest::Enabled );

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	const VertexShader* vertexShader = Material->GetVertex();
	vertexShader->FillAttributeDescriptions( attributeDescriptions );
	vertexShader->FillBindingDescription( bindingDescription );

	Pso.SetVertexLayout( bindingDescription, attributeDescriptions );

	Cache[ Hash ] = Pso.Build( Material );
	//Cache[ Hash ] = BuildPipeline();

	return Cache[ Hash ];
}