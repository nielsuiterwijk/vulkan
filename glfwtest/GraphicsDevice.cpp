#include "GraphicsDevice.h"

#include "standard.h"
#include "VulkanHelpers.h"

#include <iostream>

//TODO: Create a "VulkanRenderer" and a "GraphicDevice" class, seperating the abstract concept of a vulkan instance from the physical device handle
GraphicsDevice::GraphicsDevice():
	applicationInfo(vkDestroyInstance),
	debugCallback(applicationInfo, Vulkan::DestroyDebugReportCallbackEXT),
	device(VK_NULL_HANDLE)
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
	CreateInstance(requiredExtensions);

	if (enableValidationLayers)
	{
		HookDebugCallback();
	}

	CreateDevice();

	return true;
}

void GraphicsDevice::CreateDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(applicationInfo, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(applicationInfo, &deviceCount, devices.data());

	for (const auto& vulkanDevice : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(vulkanDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(vulkanDevice, &deviceFeatures);

		std::cout << "GPU: " << Vulkan::GetVendorName(deviceProperties.vendorID) << " " << deviceProperties.deviceName << std::endl;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			device = vulkanDevice;
			if (!FindQueueFamilies().IsComplete())
			{
				device = VK_NULL_HANDLE;
			}
		}
	}

	if (device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void GraphicsDevice::CreateInstance(const std::vector<std::string>& requiredExtensions)
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
	}
}

QueueFamilyIndices GraphicsDevice::FindQueueFamilies()
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

void GraphicsDevice::HookDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = Vulkan::DebugCallback;

	if (Vulkan::CreateDebugReportCallbackEXT(applicationInfo, &createInfo, nullptr, debugCallback.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
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
