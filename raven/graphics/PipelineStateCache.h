#pragma once

#include <unordered_map>

class Material;
class RenderPass;
class PipelineBuilder;

enum class EDepthTest
{
	Enabled,
	EnabledWithStencil,
	Disabled
};

class PipelineStateCache
{
public:
	static VkPipeline GetOrCreatePipeline( const RenderPass* pRenderPass, const Material* pMaterial, const std::vector<VkDynamicState>& dynamicStates, EDepthTest Test, glm::ivec4 ViewPort );
	static VkPipeline GetOrCreatePipeline( const PipelineBuilder& Builder, Material* Material );

	static void Destroy();
private:
	static VkPipeline BuildPipeline( const PipelineBuilder& Builder, const std::vector<VkDynamicState>& DynamicStates, Material* pMaterial );
};

//With this helper object you have an easy way to fully customize the pipeline object. Without you will get a default pipeline object.
class PipelineBuilder
{
	friend class PipelineStateCache;
public:
	PipelineBuilder();

	void SetViewport( uint32_t Width, uint32_t Height );
	void SetScissor( uint32_t Width, uint32_t Height );
	void SetViewportAndScissor( uint32_t Width, uint32_t Height );

	void SetColorBlendMode( VkBlendFactor Src, VkBlendFactor Dst );
	void SetAlphaBlendMode( VkBlendFactor Src, VkBlendFactor Dst );

	void SetCullMode( VkCullModeFlagBits Bits );
	void SetFrontFace( VkFrontFace Face );
	void SetPolygonMode( VkPolygonMode Mode );

	void SetPrimitive( VkPrimitiveTopology Topology );

	void SetDepthTest( bool Result );
	void SetDepthWrite( bool Result );
	void SetStencilTest( bool Result );

private:
	VkViewport viewport;
	VkRect2D scissor;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	//VkPipelineDynamicStateCreateInfo dynamicState;
};
