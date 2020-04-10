#pragma once

#include "graphics/memory/VulkanAllocator.h"
#include "helpers/InstanceWrapper.h"

#include <glm/fwd.hpp>

#include <string>
#include <vector>

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	void CreateInstance( std::vector<std::string> requiredExtensions );
	void HookDebugCallback();

	bool IsExtensionAvailable( const std::string& extension );

	const InstanceWrapper<VkInstance>& GetNative() const;

private:
	void CacheExtensions();
	void CacheLayers();

	bool CheckValidationLayers();

private:
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;

	InstanceWrapper<VkInstance> applicationInfo;

	InstanceWrapper<VkDebugReportCallbackEXT> debugCallback;

	//TODO: load from file
	const std::array<const char*, 1> _ValidationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
};
