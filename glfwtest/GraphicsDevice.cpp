#include "GraphicsDevice.h"

#include "standard.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include <iostream>

//TODO: Create a "VulkanRenderer" and a "GraphicDevice" class, seperating the abstract concept of a vulkan instance from the physical device handle
GraphicsDevice::GraphicsDevice():
	renderer(nullptr),
	device(VK_NULL_HANDLE)
{

}

GraphicsDevice::~GraphicsDevice()
{

}

void GraphicsDevice::Initialize(std::shared_ptr<VulkanRenderer> vulkanRenderer)
{
	renderer = vulkanRenderer;

	CreateDevice();
}

void GraphicsDevice::CreateDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(renderer->GetInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(renderer->GetInstance(), &deviceCount, devices.data());

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
