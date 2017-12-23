#pragma once

#include "helpers/InstanceWrapper.h"

#include "graphics/memory/GPUAllocator.h"

#include "shaders\Material.h"
#include "graphics\buffers\CommandBufferPool.h"
#include "graphics\PipelineStateObject.h"
#include "graphics\RenderPass.h"

#include "standard.h"

#include <vector>
#include <string>
#include <memory>
#include <mutex>

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

///Static class so everyone can access it who needs..
class GraphicsContext
{
public:
	static Allocator GlobalAllocator;
	static GPUAllocator* DeviceAllocator;

	static std::shared_ptr<CommandBufferPool> CommandBufferPoolTransient;
	static std::shared_ptr<CommandBufferPool> CommandBufferPool; //Need to use a different name..
	static std::shared_ptr<RenderPass> RenderPass;
	static std::shared_ptr<VulkanSwapChain> SwapChain;
	static std::shared_ptr<VulkanInstance> VulkanInstance;

	static VkPhysicalDevice PhysicalDevice;

	static InstanceWrapper<VkDevice> LogicalDevice;

	static QueueFamilyIndices FamilyIndices;

	static VkQueue TransportQueue;
	static VkQueue GraphicsQueue;
	static VkQueue PresentQueue;

	static glm::u32vec2 WindowSize;
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

	void Initialize(const glm::u32vec2& windowSize, std::shared_ptr<VulkanSwapChain> vulkanSwapChain);

	void SwapchainInvalidated();

	std::shared_ptr<Material> CreateMaterial(const std::string& fileName);

	void OnSwapchainInvalidated(std::function<void()> callback);

	void Lock() { busy.lock(); }
	void Unlock() { busy.unlock(); }

private:
	GraphicsDevice();

private:
	void CreatePhysicalDevice(const InstanceWrapper<VkSurfaceKHR>& surface);
	void CreateLogicalDevice();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, const InstanceWrapper<VkSurfaceKHR>&  surface);

	bool HasAllRequiredExtensions(VkPhysicalDevice device);

	void DestroySwapchain();
	void RebuildSwapchain();

private:

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME	};


	std::vector< std::function<void()> > swapchainInvalidatedCallbacks;

	std::mutex busy;



};
