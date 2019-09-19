#pragma once

#include "graphics/helpers/InstanceWrapper.h"

class VulkanSemaphore
{
public:
	VulkanSemaphore();
	~VulkanSemaphore();

	const InstanceWrapper<VkSemaphore>& GetNative() const;

private:
	InstanceWrapper<VkSemaphore> semaphore;
};
