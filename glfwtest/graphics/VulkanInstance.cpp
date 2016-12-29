#include "VulkanInstance.h"

#include "standard.h"
#include "helpers/VulkanHelpers.h"
#include "helpers/VulkanDebugHook.h"

#include <iostream>
#include <algorithm>

VulkanInstance::VulkanInstance() :
	applicationInfo(vkDestroyInstance),
	debugCallback(applicationInfo, VulkanDebug::DestroyDebugReportCallbackEXT)
{
	CacheExtensions();
	CacheLayers();

	if (enableValidationLayers && !CheckValidationLayers())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
}

VulkanInstance::~VulkanInstance()
{

}

void VulkanInstance::CreateInstance(std::vector<std::string> requiredExtensions)
{
	if (enableValidationLayers)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}


	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Raven Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Raven";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char *> requiredExtensionsC;
	requiredExtensionsC.reserve(requiredExtensions.size());
	for (int i = 0; i < requiredExtensions.size(); i++)
	{
		requiredExtensionsC.push_back(requiredExtensions[i].c_str());
	}
	createInfo.enabledExtensionCount = requiredExtensionsC.size();
	createInfo.ppEnabledExtensionNames = requiredExtensionsC.data();


	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateInstance(&createInfo, &((VkAllocationCallbacks)applicationInfo.AllocationCallbacks()), applicationInfo.Replace());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanInstance::HookDebugCallback()
{
	if (!enableValidationLayers)
	{
		return;
	}

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = VulkanDebug::DebugCallback;

	if (VulkanDebug::CreateDebugReportCallbackEXT(applicationInfo, &createInfo, &((VkAllocationCallbacks)debugCallback.AllocationCallbacks()), debugCallback.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}

bool VulkanInstance::IsExtensionAvailable(const std::string& extension)
{
	for (size_t i = 0; i < availableExtensions.size(); i++)
	{
		if (std::strcmp(availableExtensions[i].extensionName, extension.c_str()) == 0)
			return true;
	}

	return false;
}

void VulkanInstance::CacheExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	availableExtensions = std::vector<VkExtensionProperties>(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
}

void VulkanInstance::CacheLayers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	availableLayers = std::vector<VkLayerProperties>(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
}

bool VulkanInstance::CheckValidationLayers()
{
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

const InstanceWrapper<VkInstance>& VulkanInstance::Get() const
{
	return applicationInfo;
}
