#include "PipelineStateCache.h"

#include "graphics/PipelineStateObject.h"
#include "graphics/RenderPass.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "helpers/Murmur3.h"


static std::unordered_map<uint32_t, VkPipeline> Cache;

VkPipeline PipelineStateCache::GetOrCreatePipeline( std::shared_ptr<RenderPass> RenderPass, std::shared_ptr<Material> Material, const std::vector<VkDynamicState>& DynamicStates, EDepthTest DepthTest )
{
	//NU TODO: Lock this for thread safety?
	uint32_t Hash = Murmur3::Hash( RenderPass->GetHash() );
	Hash = Murmur3::Hash( Material->Hash(), Hash );
	Hash = Murmur3::Hash( (uint32_t)DepthTest, Hash );

	auto It = Cache.find( Hash );
	if ( It != Cache.end() )
	{
		return It->second;
	}

	PipelineStateObject Pso;
	Pso.Create( Material, DynamicStates, DepthTest == EDepthTest::Enabled );
	
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	std::shared_ptr<VertexShader> vertexShader = Material->GetVertex();
	vertexShader->GetAttributeDescriptions( attributeDescriptions );
	vertexShader->GetBindingDescription( bindingDescription );

	Pso.SetVertexLayout( bindingDescription, attributeDescriptions );

	Cache[ Hash ] = Pso.Build( Material );

	return Cache[ Hash ];
}