#pragma once

#include "helpers\InstanceWrapper.h"
#include "GraphicsDevice.h"

#include <vector>
#include <glm/fwd.hpp>

class VulkanInstance;
class GraphicsDevice;

struct VulkanSwapChainDetails
{
public:
	void Initialize(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);
	bool IsValid() const;

public:
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChain
{
public:
	VulkanSwapChain(const InstanceWrapper<VkInstance>& applicationInfo);
	~VulkanSwapChain();

	void Connect(const glm::u32vec2& windowSize, const VkPhysicalDevice& physicalDevice, const QueueFamilyIndices& indices, const InstanceWrapper<VkDevice>& logicalDevice);

	InstanceWrapper<VkSurfaceKHR>& GetSurface();

private:
	VkSurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync);
	VkExtent2D GetSwapExtents(const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize);

private:
	InstanceWrapper<VkSurfaceKHR> surface;
	InstanceWrapper<VkSwapchainKHR> swapChain;

	VulkanSwapChainDetails details;
};