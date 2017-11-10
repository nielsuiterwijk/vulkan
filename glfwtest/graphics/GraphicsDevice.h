#pragma once

#include "helpers/InstanceWrapper.h"

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
	static Allocator GlobalAllocator;

	static std::shared_ptr<CommandBufferPool> CommandBufferPool; //Need to use a different name..
	static std::shared_ptr<RenderPass> RenderPass;
	static std::shared_ptr<VulkanSwapChain> SwapChain;
	static std::shared_ptr<VulkanInstance> VulkanInstance;

	static VkPhysicalDevice PhysicalDevice;

	static InstanceWrapper<VkDevice> LogicalDevice;

	static QueueFamilyIndices FamilyIndices;

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

	void Initialize(const glm::u32vec2& windowSize, std::shared_ptr<VulkanInstance> vulkanRenderer, std::shared_ptr<VulkanSwapChain> vulkanSwapChain);

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

private:

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME	};


	std::vector< std::function<void()> > swapchainInvalidatedCallbacks;

	std::mutex busy;



};
