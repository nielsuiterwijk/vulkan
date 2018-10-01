#include "GraphicsContext.h"

VkPhysicalDevice GraphicsContext::PhysicalDevice = VK_NULL_HANDLE;

VulkanAllocator GraphicsContext::GlobalAllocator;
GPUAllocator* GraphicsContext::DeviceAllocator = nullptr;

std::shared_ptr<CommandBufferPool> GraphicsContext::CommandBufferPoolTransient = nullptr;
std::shared_ptr<CommandBufferPool> GraphicsContext::CommandBufferPool = nullptr;
std::shared_ptr<RenderPass> GraphicsContext::RenderPass = nullptr;
std::shared_ptr<VulkanSwapChain> GraphicsContext::SwapChain = nullptr;
std::shared_ptr<VulkanInstance> GraphicsContext::VulkanInstance = nullptr;
std::shared_ptr<VulkanDescriptorPool> GraphicsContext::DescriptorPool = nullptr;

InstanceWrapper<VkPipelineLayout> GraphicsContext::PipelineLayout = { GraphicsContext::LogicalDevice, vkDestroyPipelineLayout, GraphicsContext::GlobalAllocator.Get() };
InstanceWrapper<VkDevice> GraphicsContext::LogicalDevice = { vkDestroyDevice, GraphicsContext::GlobalAllocator.Get() };

QueueFamilyIndices GraphicsContext::FamilyIndices = {};

VkEvent GraphicsContext::TransportEvent = {};
VulkanSemaphore* GraphicsContext::TransportSemaphore = nullptr;
std::mutex GraphicsContext::QueueLock = {};

VkQueue GraphicsContext::TransportQueue = {};
VkQueue GraphicsContext::GraphicsQueue = {};
VkQueue GraphicsContext::PresentQueue = {};

glm::u32vec2 GraphicsContext::WindowSize = glm::uvec2(0, 0);
