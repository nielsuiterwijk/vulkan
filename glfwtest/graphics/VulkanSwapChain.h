#pragma once

#include "helpers\InstanceWrapper.h"
#include "GraphicsDevice.h"
#include "RenderPass.h"

#include <vector>
#include <glm/fwd.hpp>

class VulkanInstance;
class GraphicsDevice;

struct VulkanSwapChainDetails
{
public:
	void Initialize(const VkPhysicalDevice& physicalDevice, const InstanceWrapper<VkSurfaceKHR>& surface);
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

	void Connect(const glm::u32vec2& windowSize, const QueueFamilyIndices& indices);

	void SetupFrameBuffers(std::shared_ptr<RenderPass> renderPass);

	InstanceWrapper<VkSurfaceKHR>& GetSurface();

	const VkSurfaceFormatKHR& GetSurfaceFormat() const;
	const VkPresentModeKHR& GetPresentMode() const;

private:
	VkSurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR PickSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync);
	VkExtent2D GetSwapExtents(const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize);

private:
	InstanceWrapper<VkSurfaceKHR> surface;
	InstanceWrapper<VkSwapchainKHR> swapChain;

	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;

	VulkanSwapChainDetails details;

	std::vector<VkImage> images;
	std::vector< InstanceWrapper<VkImageView> > imageViews;
	std::vector< InstanceWrapper<VkFramebuffer> > framebuffers;

	VkFormat imageFormat;
	VkExtent2D extent;

};