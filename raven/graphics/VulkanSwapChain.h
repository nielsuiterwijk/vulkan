#pragma once

#include "GraphicsDevice.h"
#include "RenderPass.h"
#include "graphics/FrameBuffer.h"
#include "graphics\DepthBuffer.h"
#include "graphics\buffers\CommandBuffer.h"
#include "graphics\helpers\VulkanSemaphore.h"
#include "graphics\textures\Texture2D.h"
#include "helpers\InstanceWrapper.h"

#include <glm/fwd.hpp>
#include <vector>

class VulkanInstance;
class GraphicsDevice;

struct VulkanSwapChainDetails
{
public:
	void Initialize( const VkPhysicalDevice& physicalDevice, const InstanceWrapper<VkSurfaceKHR>& surface );
	bool IsValid() const;

public:
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChain
{
public:
	VulkanSwapChain();
	~VulkanSwapChain();

	void Connect( const glm::u32vec2& windowSize, const QueueFamilyIndices& indices );

	void SetupFrameBuffers();

	void DestroyFrameBuffers();
	void DestroySwapchain();

	//Returns the backbuffer index
	int32_t PrepareBackBuffer();

	const DepthBuffer& GetDepthBuffer() const;

	InstanceWrapper<VkSurfaceKHR>& GetSurface();
	const FrameBuffer& GetFrameBuffer( int32_t frameIndex );

	const InstanceWrapper<VkSwapchainKHR>& GetNative() const;
	const VkSurfaceFormatKHR& GetSurfaceFormat() const;
	const VkPresentModeKHR& GetPresentMode() const;
	VkExtent2D GetExtent() const;

	uint32_t GetAmountOfFrameBuffers() const { return static_cast<uint32_t>( backBuffers.size() ); }

private:
	VkSurfaceFormatKHR PickSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats );
	VkPresentModeKHR PickSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync );
	VkExtent2D GetSwapExtents( const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize );

private:
	InstanceWrapper<VkSurfaceKHR> surface;
	InstanceWrapper<VkSwapchainKHR> swapChain;

	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;

	VulkanSwapChainDetails details;

	std::vector<FrameBuffer> backBuffers;
	DepthBuffer depthBuffer;

	VkFormat imageFormat;
	VkExtent2D extent;

	int32_t nextBackBufferIndex;
	int32_t currentBackBufferIndex;
};
