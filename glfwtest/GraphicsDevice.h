#pragma once

#include "InstanceWrapper.h"

#include <vector>
#include <string>

struct QueueFamilyIndices
{
	int graphicsFamily = -1;

	bool IsComplete()
	{
		return graphicsFamily >= 0;
	}
};

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

	void CreateInstance(const std::vector<std::string>& requiredExtensions);
	void HookDebugCallback();
	void CreateDevice();
	QueueFamilyIndices FindQueueFamilies();

private:
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;

	VkPhysicalDevice device;

	InstanceWrapper<VkInstance> applicationInfo;
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
