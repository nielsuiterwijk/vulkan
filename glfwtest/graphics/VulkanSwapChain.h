#pragma once

#include "helpers\InstanceWrapper.h"
#include "GraphicsDevice.h"
#include "graphics\buffers\CommandBuffer.h"
#include "graphics\helpers\VulkanSemaphore.h"
#include "graphics\textures\Texture2D.h"
#include "graphics\DepthBuffer.h"
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
	Texture2D texture;
	InstanceWrapper<VkFramebuffer> framebuffer;

	VulkanSemaphore semaphore;	
};

class VulkanSwapChain
{
public:
	VulkanSwapChain();
	~VulkanSwapChain();
	
	void Connect(const glm::u32vec2& windowSize, const QueueFamilyIndices& indices);

	void SetupFrameBuffers();

	void DestroyFrameBuffers();
	void DestroySwapchain();

	//Returns the backbuffer index
	int32_t PrepareBackBuffer();

	const DepthBuffer& GetDepthBuffer() const;

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
	DepthBuffer depthBuffer;
	
	VkFormat imageFormat;
	VkExtent2D extent;

	int32_t nextBackBufferIndex;
	int32_t currentBackBufferIndex;
};