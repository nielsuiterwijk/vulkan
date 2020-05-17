#pragma once

#include <unordered_map>

class Material;
class RenderPass;

enum class EDepthTest
{
	Enabled,
	Disabled
};

class PipelineStateCache
{
public:
	static VkPipeline GetOrCreatePipeline( std::shared_ptr<RenderPass> RenderPass, std::shared_ptr<Material> Material, const std::vector<VkDynamicState>& dynamicStates, EDepthTest Test);
};