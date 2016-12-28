#pragma once

#include "helpers/InstanceWrapper.h"

#include <vector>
#include <string>
#include <memory>

class VulkanRenderer;

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool IsComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};



class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(std::shared_ptr<VulkanRenderer> vulkanRenderer);



private:
	void CreatePhysicalDevice();
	void CreateLogicalDevice(const QueueFamilyIndices & indices);

	void CreateSwapChain();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

	bool HasAllRequiredExtensions(VkPhysicalDevice device);

private:
	std::shared_ptr<VulkanRenderer> renderer;

	VkPhysicalDevice physicalDevice;

	InstanceWrapper<VkDevice> logicalDevice { vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME	};

};
