#pragma once

#include "helpers/InstanceWrapper.h"

#include "shaders\Material.h"

#include "standard.h"

#include <vector>
#include <string>
#include <memory>

class VulkanInstance;
class VulkanSwapChain;


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
	GraphicsDevice(glm::u32vec2 windowSize);
	~GraphicsDevice();

	void Initialize(std::shared_ptr<VulkanInstance> vulkanRenderer, std::shared_ptr<VulkanSwapChain> vulkanSwapChain);

	const VkPhysicalDevice& GetPhysicalDevice() const;
	const VkDevice& GetDevice() const;
	std::shared_ptr<VulkanSwapChain> GetSwapChain() const;

	std::shared_ptr<Material> CreateMaterial(const std::string& fileName);

private:
	void CreatePhysicalDevice(const VkSurfaceKHR& surface);
	void CreateLogicalDevice(const QueueFamilyIndices & indices);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface);

	bool HasAllRequiredExtensions(VkPhysicalDevice device);

private:
	std::shared_ptr<VulkanInstance> vulkanInstance;
	std::shared_ptr<VulkanSwapChain> swapChain;

	VkPhysicalDevice physicalDevice;

	InstanceWrapper<VkDevice> logicalDevice { vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME	};

	glm::u32vec2 windowSize;

};
