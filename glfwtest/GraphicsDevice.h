#pragma once

#include "InstanceWrapper.h"

#include <vector>

class GraphicsDevice
{
public:
	GraphicsDevice();

	~GraphicsDevice();

	bool IsExtensionAvailable(std::string extension);

private:
	std::vector<VkExtensionProperties> availableExtensions;

	InstanceWrapper<VkInstance> applicationInfo { vkDestroyInstance };

};
