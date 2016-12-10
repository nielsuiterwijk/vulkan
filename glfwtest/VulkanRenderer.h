#pragma once

#include "InstanceWrapper.h"

#include <vector>
#include <string>

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void CreateInstance(std::vector<std::string> requiredExtensions);
	void HookDebugCallback();

	bool IsExtensionAvailable(const std::string& extension);

	InstanceWrapper<VkInstance>& GetInstance();
	InstanceWrapper<VkSurfaceKHR>& GetSurface();

private:
	void CacheExtensions();
	void CacheLayers();

	bool CheckValidationLayers();



private:
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;

	InstanceWrapper<VkInstance> applicationInfo;
	InstanceWrapper<VkSurfaceKHR> surface { applicationInfo, vkDestroySurfaceKHR };

	InstanceWrapper<VkDebugReportCallbackEXT> debugCallback;

	//TODO: load from file
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