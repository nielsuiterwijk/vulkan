#include "GraphicsDevice.h"

#include "standard.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include <iostream>
#include <set>

//TODO: Create a "VulkanRenderer" and a "GraphicDevice" class, seperating the abstract concept of a vulkan instance from the physical device handle
GraphicsDevice::GraphicsDevice():
	renderer(nullptr),
	physicalDevice(VK_NULL_HANDLE)
{

}

GraphicsDevice::~GraphicsDevice()
{

}

void GraphicsDevice::Initialize(std::shared_ptr<VulkanRenderer> vulkanRenderer)
{
	renderer = vulkanRenderer;

	CreatePhysicalDevice();

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	CreateLogicalDevice(indices);

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);

	CreateSwapChain();
}

void GraphicsDevice::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = renderer->QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
}

void GraphicsDevice::CreateLogicalDevice(const QueueFamilyIndices& indices)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//No need for now
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//Note: logical device validation layers got deprecated see: https://www.khronos.org/registry/vulkan/specs/1.0-extensions/xhtml/vkspec.html 31.1.1 Device Layer Deprecation
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, logicalDevice.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}
}

void GraphicsDevice::CreatePhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(renderer->GetInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(renderer->GetInstance(), &deviceCount, devices.data());

	physicalDevice = VK_NULL_HANDLE;

	for (const auto& vulkanPhysicalDevice : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(vulkanPhysicalDevice, &deviceFeatures);

		std::cout << "GPU: " << Vulkan::GetVendorName(deviceProperties.vendorID) << " " << deviceProperties.deviceName << std::endl;

		bool hasValidQueues = FindQueueFamilies(vulkanPhysicalDevice).IsComplete();
		bool hasValidExtensions = HasAllRequiredExtensions(vulkanPhysicalDevice);
		bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool hasValidSwapchain = false;

		if (hasValidExtensions)
		{
			SwapChainSupportDetails swapChainSupport = renderer->QuerySwapChainSupport(vulkanPhysicalDevice);
			hasValidSwapchain = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}


		std::cout << "hasValidQueues: " << hasValidQueues << " hasValidExtensions: " << hasValidExtensions << " isDiscreteGPU: " << isDiscreteGPU << " hasValidSwapchain: " << hasValidSwapchain << std::endl;

		if (hasValidQueues && hasValidExtensions && isDiscreteGPU && hasValidSwapchain)
		{
			physicalDevice = vulkanPhysicalDevice;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

QueueFamilyIndices GraphicsDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, renderer->GetSurface(), &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

bool GraphicsDevice::HasAllRequiredExtensions(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

