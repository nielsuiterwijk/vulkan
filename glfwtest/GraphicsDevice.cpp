#include "GraphicsDevice.h"

#include "standard.h"

#include <iostream>

GraphicsDevice::GraphicsDevice()
{
	CacheExtensions();
	CacheLayers();

	if (enableValidationLayers && !CheckValidationLayers())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
}

GraphicsDevice::~GraphicsDevice()
{

}

bool GraphicsDevice::CreateVulkanInstance(const std::vector<std::string>& requiredExtensions)
{
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

	VkResult result = vkCreateInstance(&createInfo, nullptr, applicationInfo.Replace());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
		return false;
	}

	return true;
}

bool GraphicsDevice::IsExtensionAvailable(std::string extension)
{
	for (size_t i = 0; i < availableExtensions.size(); i++)
	{
		if (std::strcmp(availableExtensions[i].extensionName, extension.c_str()) == 0)
			return true;
	}

	return false;
}

std::vector<std::string> GraphicsDevice::GetRequiredInstanceExtensions()
{
	if (enableValidationLayers)
	{
		return std::vector<std::string> { VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
	}
	else
	{
		return std::vector<std::string>();
	}
}

void GraphicsDevice::CacheExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	availableExtensions = std::vector<VkExtensionProperties>(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
}

void GraphicsDevice::CacheLayers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	availableLayers = std::vector<VkLayerProperties>(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
}

bool GraphicsDevice::CheckValidationLayers()
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