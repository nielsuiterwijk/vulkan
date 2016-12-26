#pragma once

#include "InstanceWrapper.h"

#include <glm/fwd.hpp>

#include <vector>
#include <string>

//TODO: Create a Swapchain class?
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

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

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

private:
	void CacheExtensions();
	void CacheLayers();

	bool CheckValidationLayers();

	//TODO: SwapChain class?
	VkSurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync);
	VkExtent2D GetSwapExtents(const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize);

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