#pragma once

#include "graphics\GraphicsDevice.h"
#include "graphics\RenderPass.h"
#include "graphics\VulkanInstance.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\buffers\CommandBufferPool.h"
#include "graphics\helpers\InstanceWrapper.h"
#include "graphics\memory\GPUAllocator.h"

///Static class so everyone can access it who needs..
class GraphicsContext
{
public:
	static VulkanAllocator GlobalAllocator;
	static std::unique_ptr<GPUAllocator> DeviceAllocator;

	static std::shared_ptr<CommandBufferPool> CommandBufferPool; //Need to use a different name..

	static std::shared_ptr<RenderPass> RenderPass;
	static std::shared_ptr<VulkanSwapChain> SwapChain;
	static std::shared_ptr<VulkanInstance> VulkanInstance;
	static std::shared_ptr<VulkanDescriptorPool> DescriptorPool;

	static VkPhysicalDevice PhysicalDevice;

	static InstanceWrapper<VkPipelineLayout> PipelineLayout;
	static InstanceWrapper<VkDevice> LogicalDevice;

	static QueueFamilyIndices FamilyIndices;

	static VkEvent TransportEvent;
	static VulkanSemaphore* TransportSemaphore;
	static Mutex QueueLock;

	static VkQueue TransportQueue;
	static VkQueue GraphicsQueue;
	static VkQueue PresentQueue;

	static glm::u32vec2 WindowSize;
};
