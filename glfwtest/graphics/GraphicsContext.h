#pragma once

#include "standard.h"

#include "graphics\buffers\CommandBufferPool.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\helpers\InstanceWrapper.h"
#include "graphics\memory\GPUAllocator.h"
#include "graphics\RenderPass.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\VulkanInstance.h"

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
	static std::shared_ptr<VulkanDescriptorPool> DescriptorPool;

	static VkPhysicalDevice PhysicalDevice;

	static InstanceWrapper<VkPipelineLayout> PipelineLayout;
	static InstanceWrapper<VkDevice> LogicalDevice;

	static QueueFamilyIndices FamilyIndices;

	static VkEvent TransportEvent;
	static VulkanSemaphore* TransportSemaphore;
	static std::mutex TransportQueueLock;

	static VkQueue TransportQueue;
	static VkQueue GraphicsQueue;
	static VkQueue PresentQueue;

	static glm::u32vec2 WindowSize;
};