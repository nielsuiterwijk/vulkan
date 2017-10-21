#pragma once

#include "helpers\InstanceWrapper.h"
#include "GraphicsDevice.h"
#include "graphics\buffers\CommandBuffer.h"
#include "graphics\helpers\VulkanSemaphore.h"
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

struct SwapChainSurface
{
	VkImage image;
	InstanceWrapper<VkImageView> imageView;
	InstanceWrapper<VkFramebuffer> framebuffer;

	VulkanSemaphore semaphore;	
};

class VulkanSwapChain
{
public:
	VulkanSwapChain(const InstanceWrapper<VkInstance>& applicationInfo);
	~VulkanSwapChain();

	void Connect(const glm::u32vec2& windowSize, const QueueFamilyIndices& indices);

	void SetupFrameBuffers();

	//Returns the backbuffer index
	int32_t PrepareBackBuffer();

	InstanceWrapper<VkSurfaceKHR>& GetSurface();
	const SwapChainSurface& GetFrameBuffer(int32_t frameIndex);

	const InstanceWrapper<VkSwapchainKHR>& GetNative() const;
	const VkSurfaceFormatKHR& GetSurfaceFormat() const;
	const VkPresentModeKHR& GetPresentMode() const;
	VkExtent2D GetExtent() const;

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

	std::vector<SwapChainSurface> backBuffers;
	
	VkFormat imageFormat;
	VkExtent2D extent;

	int32_t nextBackBufferIndex;
	int32_t currentBackBufferIndex;
};