#pragma once

#include <vulkan\vulkan.h>
#include "graphics\helpers\InstanceWrapper.h"

class VulkanSemaphore
{
public:
	VulkanSemaphore() : 
		semaphore(GraphicsContext::LogicalDevice, vkDestroySemaphore, GraphicsContext::GlobalAllocator.Get())
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(GraphicsContext::LogicalDevice, &semaphoreInfo, semaphore.AllocationCallbacks(), semaphore.Replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}

	~VulkanSemaphore()
	{
		semaphore = nullptr;
	}
	
	const InstanceWrapper<VkSemaphore>& GetNative() const { return semaphore; }

private:
	InstanceWrapper<VkSemaphore> semaphore;
};
