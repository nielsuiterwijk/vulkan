#pragma once

#include "standard.h"

class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool(uint32_t count);
	~VulkanDescriptorPool();

	const VkDescriptorPool& Get() const;

private:
	VkDescriptorPool descriptorPool;
};