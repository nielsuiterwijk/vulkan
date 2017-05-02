#include "GraphicsDevice.h"

#include "standard.h"
#include "helpers/VulkanHelpers.h"
#include "VulkanInstance.h"
#include "VulkanSwapChain.h"

#include <iostream>
#include <set>

GraphicsDevice::GraphicsDevice(glm::u32vec2 windowSize) :
	vulkanInstance(nullptr),
	physicalDevice(VK_NULL_HANDLE),
	windowSize(windowSize)
{

}

GraphicsDevice::~GraphicsDevice()
{
	swapChain = nullptr;
	logicalDevice = nullptr;
	vulkanInstance = nullptr;

}

void GraphicsDevice::Initialize(std::shared_ptr<VulkanInstance> vulkanRenderer, std::shared_ptr<VulkanSwapChain> vulkanSwapChain)
{
	vulkanInstance = vulkanRenderer;
	swapChain = vulkanSwapChain;

	CreatePhysicalDevice(swapChain->GetSurface());

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, swapChain->GetSurface());

	CreateLogicalDevice(indices);

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);

	swapChain->Connect(windowSize, physicalDevice, indices, logicalDevice);

	std::shared_ptr<Material> fixedMaterial = CreateMaterial("fixed");
}

std::shared_ptr<Material> GraphicsDevice::CreateMaterial(const std::string& fileName)
{
	std::shared_ptr<Material> material = std::make_shared<Material>(fileName, logicalDevice);

	return material;
}

const VkPhysicalDevice& GraphicsDevice::GetPhysicalDevice() const
{
	return physicalDevice;
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

	if (vkCreateDevice(physicalDevice, &createInfo, logicalDevice.AllocationCallbacks(), logicalDevice.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}
}

void GraphicsDevice::CreatePhysicalDevice(const VkSurfaceKHR& surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanInstance->Get(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vulkanInstance->Get(), &deviceCount, devices.data());

	physicalDevice = VK_NULL_HANDLE;

	for (const auto& vulkanPhysicalDevice : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(vulkanPhysicalDevice, &deviceFeatures);

		std::cout << "GPU: " << Vulkan::GetVendorName(deviceProperties.vendorID) << " " << deviceProperties.deviceName << std::endl;

		bool hasValidExtensions = HasAllRequiredExtensions(vulkanPhysicalDevice);

		bool hasValidQueues = FindQueueFamilies(vulkanPhysicalDevice, surface).IsComplete();
		bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool hasValidSwapchain = false;

		if (hasValidExtensions)
		{
			VulkanSwapChainDetails swapChainDetails;
			swapChainDetails.Initialize(vulkanPhysicalDevice, surface);
			hasValidSwapchain = swapChainDetails.IsValid();
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

QueueFamilyIndices GraphicsDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface)
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
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

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


std::shared_ptr<VulkanSwapChain> GraphicsDevice::GetSwapChain() const
{
	return swapChain;
}