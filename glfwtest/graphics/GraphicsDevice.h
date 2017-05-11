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

///Static class so everyone can access it who needs..
class GraphicsContext
{
public:

	static VkPhysicalDevice PhysicalDevice;

	static InstanceWrapper<VkDevice> LogicalDevice;

	static VkQueue GraphicsQueue;
	static VkQueue PresentQueue;

	static glm::u32vec2 WindowSize;
};

class GraphicsDevice
{
public:
	GraphicsDevice(glm::u32vec2 windowSize);
	~GraphicsDevice();

	void Initialize(std::shared_ptr<VulkanInstance> vulkanRenderer, std::shared_ptr<VulkanSwapChain> vulkanSwapChain);

	std::shared_ptr<VulkanSwapChain> GetSwapChain() const;

	std::shared_ptr<Material> CreateMaterial(const std::string& fileName);

	void DestroySwapChain()
	{
		swapChain = nullptr;
	}

private:
	void CreatePhysicalDevice(const InstanceWrapper<VkSurfaceKHR>& surface);
	void CreateLogicalDevice(const QueueFamilyIndices & indices);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, const InstanceWrapper<VkSurfaceKHR>&  surface);

	bool HasAllRequiredExtensions(VkPhysicalDevice device);

private:
	std::shared_ptr<VulkanInstance> vulkanInstance;
	std::shared_ptr<VulkanSwapChain> swapChain;

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME	};

};
