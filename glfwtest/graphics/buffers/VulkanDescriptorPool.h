#pragma once

#include "graphics/helpers/InstanceWrapper.h"
#include "standard.h"

class UniformBuffer;
class Texture2D;
class TextureSampler;
class Material;
class VertexShader;
class FragmentShader;

class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool();
	~VulkanDescriptorPool();

	void SetupBindings( std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<FragmentShader> pFragmentShader );

	VkDescriptorPool GetNative() const;

	VkDescriptorSet RetrieveDescriptorSet( std::shared_ptr<Material> material, Texture2D* texture, TextureSampler* sampler );
	VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }

	size_t GetCurrentIndex() const { return currentIndex; }

private:
	void CreatePoolAndSets();

private:
	InstanceWrapper<VkDescriptorPool> descriptorPool;
	InstanceWrapper<VkDescriptorSetLayout> descriptorSetLayout;
	InstanceWrapper<VkPipelineLayout> pipelineLayout;

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorPoolSize> poolSizes;

	size_t currentIndex;
};
