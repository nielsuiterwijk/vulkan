#pragma once

#include "standard.h"
#include "graphics/helpers/InstanceWrapper.h"

class UniformBuffer;
class Texture2D;
class TextureSampler;
class Material;

class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool(uint32_t count);
	~VulkanDescriptorPool();

	VkDescriptorPool GetNative() const;

	VkDescriptorSet GetDescriptorSet(std::shared_ptr<Material> material, Texture2D* texture, TextureSampler* sampler);

	uint32_t GetCurrentIndex() const { return currentIndex; }

private:
	void CreateLayouts();
	void CreatePoolAndSets(uint32_t count);

private:
	InstanceWrapper<VkDescriptorPool> descriptorPool;
	InstanceWrapper<VkDescriptorSetLayout> descriptorSetLayout;

	std::vector<VkDescriptorSet> descriptorSets;

	uint32_t currentIndex;
};