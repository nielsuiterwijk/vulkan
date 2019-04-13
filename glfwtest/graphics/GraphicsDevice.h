#pragma once

#include "helpers/InstanceWrapper.h"

#include "graphics/memory/GPUAllocator.h"

#include "graphics\PipelineStateObject.h"
#include "graphics\RenderPass.h"
#include "graphics\buffers\CommandBufferPool.h"
#include "shaders\Material.h"

#include "standard.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class VulkanInstance;
class VulkanSwapChain;

struct QueueFamilyIndices
{
	int transportFamily = -1;
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool IsComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0 && transportFamily >= 0;
	}
};

class GraphicsDevice
{
public:
	static GraphicsDevice& Instance()
	{
		static GraphicsDevice instance;

		return instance;
	}

public:
	~GraphicsDevice();

	void Finalize();

	void Initialize( const glm::u32vec2& windowSize, std::shared_ptr<VulkanSwapChain> vulkanSwapChain );

	void SwapchainInvalidated();

	std::shared_ptr<Material> CreateMaterial( const std::string& fileName );

	void OnSwapchainInvalidated( std::function<void()> callback );

	void Lock() { busy.lock(); }
	void Unlock() { busy.unlock(); }

private:
	GraphicsDevice();

private:
	void CreatePhysicalDevice( const InstanceWrapper<VkSurfaceKHR>& surface );
	void CreateLogicalDevice();

	QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice physicalDevice, const InstanceWrapper<VkSurfaceKHR>& surface );

	bool HasAllRequiredExtensions( VkPhysicalDevice device );

	void DestroySwapchain();
	void RebuildSwapchain();

	void CreateDescriptorPool();

private:
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	std::vector<std::function<void()>> swapchainInvalidatedCallbacks;

	std::mutex busy;
};
