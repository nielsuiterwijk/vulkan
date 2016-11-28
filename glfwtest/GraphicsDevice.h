#pragma once

#include "InstanceWrapper.h"

#include <vector>
#include <string>

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	bool CreateVulkanInstance(const std::vector<std::string>& requiredExtensions);

	bool IsExtensionAvailable(std::string extension);

	std::vector<std::string> GetRequiredInstanceExtensions();

private:
	void CacheExtensions();
	void CacheLayers();

	bool CheckValidationLayers();

private:
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;

	InstanceWrapper<VkInstance> applicationInfo { vkDestroyInstance };

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

};
