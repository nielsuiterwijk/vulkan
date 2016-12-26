#include "VulkanRenderer.h"

#include "standard.h"
#include "VulkanHelpers.h"
#include "VulkanDebugHook.h"

#include <iostream>
#include <algorithm>

//TODO: Create a "VulkanRenderer" and a "GraphicDevice" class, seperating the abstract concept of a vulkan instance from the physical device handle
VulkanRenderer::VulkanRenderer() :
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

VulkanRenderer::~VulkanRenderer()
{

}

void VulkanRenderer::CreateInstance(std::vector<std::string> requiredExtensions)
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

	VkResult result = vkCreateInstance(&createInfo, nullptr, applicationInfo.Replace());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanRenderer::HookDebugCallback()
{
	if (!enableValidationLayers)
	{
		return;
	}

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = VulkanDebug::DebugCallback;

	if (VulkanDebug::CreateDebugReportCallbackEXT(applicationInfo, &createInfo, nullptr, debugCallback.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}

bool VulkanRenderer::IsExtensionAvailable(const std::string& extension)
{
	for (size_t i = 0; i < availableExtensions.size(); i++)
	{
		if (std::strcmp(availableExtensions[i].extensionName, extension.c_str()) == 0)
			return true;
	}

	return false;
}

void VulkanRenderer::CacheExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	availableExtensions = std::vector<VkExtensionProperties>(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
}

void VulkanRenderer::CacheLayers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	availableLayers = std::vector<VkLayerProperties>(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
}

bool VulkanRenderer::CheckValidationLayers()
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


SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	SwapChainSupportDetails details = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, GetSurface(), &details.capabilities);

	//TODO: See if there is a way to make these sort of calls a helper method?
	{
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, GetSurface(), &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, GetSurface(), &formatCount, details.formats.data());
		}
	}

	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, GetSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, GetSurface(), &presentModeCount, details.presentModes.data());
		}
	}

	return details;
}


VkSurfaceFormatKHR VulkanRenderer::GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//Best case scenario, our surface accepts any format, lets pick the best combination
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//If we cannot choose any, lets see if there is a 'best' format available.
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	//If thats not possible, don't bother and use whatever.
	return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync)
{
	//These are explained here: https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html
	if (waitForVSync)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
	}
	else
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				return availablePresentMode;
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::GetSwapExtents(const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize)
{
	//The idea is to always pick the size of the window we render to, if that is not possible (told by have the current extent set to int32.max)
	//we'll try the largest size available.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { windowSize.x, windowSize.y };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}


InstanceWrapper<VkInstance>& VulkanRenderer::GetInstance()
{
	return applicationInfo;
}

InstanceWrapper<VkSurfaceKHR>& VulkanRenderer::GetSurface()
{
	return surface;
}