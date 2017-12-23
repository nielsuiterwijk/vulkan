#include "GraphicsDevice.h"

#include "PipelineStateObject.h"
#include "RenderPass.h"
#include "shaders\ShaderCache.h"
#include "standard.h"
#include "helpers/VulkanHelpers.h"
#include "VulkanInstance.h"
#include "VulkanSwapChain.h"

#include "memory/GPUAllocator.h"

#include <iostream>
#include <set>

//TODO: Seperate file?

VkPhysicalDevice GraphicsContext::PhysicalDevice = VK_NULL_HANDLE;

Allocator GraphicsContext::GlobalAllocator;
GPUAllocator* GraphicsContext::DeviceAllocator = nullptr;

std::shared_ptr<CommandBufferPool> GraphicsContext::CommandBufferPoolTransient = nullptr;
std::shared_ptr<CommandBufferPool> GraphicsContext::CommandBufferPool = nullptr;
std::shared_ptr<RenderPass> GraphicsContext::RenderPass = nullptr;
std::shared_ptr<VulkanSwapChain> GraphicsContext::SwapChain = nullptr;
std::shared_ptr<VulkanInstance> GraphicsContext::VulkanInstance = nullptr;

InstanceWrapper<VkDevice> GraphicsContext::LogicalDevice = { vkDestroyDevice, GraphicsContext::GlobalAllocator.Get() };

QueueFamilyIndices GraphicsContext::FamilyIndices = {};

VkQueue GraphicsContext::TransportQueue = {};
VkQueue GraphicsContext::GraphicsQueue = {};
VkQueue GraphicsContext::PresentQueue = {};

glm::u32vec2 GraphicsContext::WindowSize = glm::uvec2(0, 0);


GraphicsDevice::GraphicsDevice()
{
}

void GraphicsDevice::Finalize()
{
	if (GraphicsContext::SwapChain == nullptr)
		return;

	GraphicsContext::SwapChain->DestroyFrameBuffers();

	GraphicsContext::CommandBufferPoolTransient->Clear();
	GraphicsContext::CommandBufferPoolTransient = nullptr;

	GraphicsContext::CommandBufferPool->Clear();
	GraphicsContext::CommandBufferPool = nullptr;

	GraphicsContext::RenderPass = nullptr;
	ShaderCache::Destroy();

	GraphicsContext::SwapChain->DestroySwapchain();

	delete GraphicsContext::DeviceAllocator;

	GraphicsContext::LogicalDevice = nullptr;
	//a = nullptr;

	GraphicsContext::SwapChain = nullptr;
	GraphicsContext::VulkanInstance = nullptr;
}

GraphicsDevice::~GraphicsDevice()
{
	Finalize();
}

void GraphicsDevice::Initialize(const glm::u32vec2& windowSize, std::shared_ptr<VulkanSwapChain> vulkanSwapChain)
{
	GraphicsContext::WindowSize = windowSize;
	GraphicsContext::SwapChain = vulkanSwapChain;

	CreatePhysicalDevice(GraphicsContext::SwapChain->GetSurface());
	
	GraphicsContext::FamilyIndices = FindQueueFamilies(GraphicsContext::PhysicalDevice, GraphicsContext::SwapChain->GetSurface());

	CreateLogicalDevice();

	GraphicsContext::DeviceAllocator = new GPUAllocator(16 * 1024 * 1024, 8);

	vkGetDeviceQueue(GraphicsContext::LogicalDevice, GraphicsContext::FamilyIndices.transportFamily, 0, &GraphicsContext::TransportQueue);
	vkGetDeviceQueue(GraphicsContext::LogicalDevice, GraphicsContext::FamilyIndices.graphicsFamily, 0, &GraphicsContext::GraphicsQueue);
	vkGetDeviceQueue(GraphicsContext::LogicalDevice, GraphicsContext::FamilyIndices.presentFamily, 0, &GraphicsContext::PresentQueue);

	GraphicsContext::SwapChain->Connect(GraphicsContext::WindowSize, GraphicsContext::FamilyIndices);

	GraphicsContext::RenderPass = std::make_shared<RenderPass>(GraphicsContext::SwapChain->GetSurfaceFormat().format);
	GraphicsContext::SwapChain->SetupFrameBuffers();

	GraphicsContext::CommandBufferPool = std::make_shared<CommandBufferPool>(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	GraphicsContext::CommandBufferPoolTransient = std::make_shared<CommandBufferPool>(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	
}


void GraphicsDevice::DestroySwapchain()
{
	//destroy Framebuffer
	GraphicsContext::SwapChain->DestroyFrameBuffers();
	//Free CommandBuffers
	GraphicsContext::CommandBufferPool->FreeAll();

	//Destroy Pipeline, Pipeline layout and RenderPass
	GraphicsContext::RenderPass = nullptr;

	//Destroy surfaces
	//Destroy swapchain OR pass it along to the new swapchain
	GraphicsContext::SwapChain->DestroySwapchain();
}

void GraphicsDevice::RebuildSwapchain()
{
	//rebuild:
	//createSwapChain();
	//createImageViews();
	GraphicsContext::SwapChain->Connect(GraphicsContext::WindowSize, GraphicsContext::FamilyIndices);
	//createRenderPass();
	GraphicsContext::RenderPass = std::make_shared<RenderPass>(GraphicsContext::SwapChain->GetSurfaceFormat().format);
	GraphicsContext::SwapChain->SetupFrameBuffers();

	//createCommandBuffers();
	GraphicsContext::CommandBufferPool->RecreateAll();
}

void GraphicsDevice::SwapchainInvalidated()
{
	Lock();
	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);

	DestroySwapchain();
	RebuildSwapchain();

	for (size_t i = 0; i < swapchainInvalidatedCallbacks.size(); i++)
	{
		swapchainInvalidatedCallbacks[i]();
	}
	

	Unlock();
}

std::shared_ptr<Material> GraphicsDevice::CreateMaterial(const std::string& fileName)
{
	std::shared_ptr<Material> material = std::make_shared<Material>(fileName);

	return material;
}

void GraphicsDevice::OnSwapchainInvalidated(std::function<void()> callback)
{
	swapchainInvalidatedCallbacks.push_back(callback);
}

void GraphicsDevice::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { GraphicsContext::FamilyIndices.graphicsFamily, GraphicsContext::FamilyIndices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//No need for now
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//Note: logical device validation layers got deprecated see: https://www.khronos.org/registry/vulkan/specs/1.0-extensions/xhtml/vkspec.html 31.1.1 Device Layer Deprecation
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(GraphicsContext::PhysicalDevice, &createInfo, GraphicsContext::LogicalDevice.AllocationCallbacks(), GraphicsContext::LogicalDevice.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}
}

void GraphicsDevice::CreatePhysicalDevice(const InstanceWrapper<VkSurfaceKHR>&  surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(GraphicsContext::VulkanInstance->GetNative(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(GraphicsContext::VulkanInstance->GetNative(), &deviceCount, devices.data());

	GraphicsContext::PhysicalDevice = VK_NULL_HANDLE;

	for (const auto& vulkanPhysicalDevice : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(vulkanPhysicalDevice, &deviceFeatures);

		std::cout << "Vendor: " << Vulkan::GetVendorName(deviceProperties.vendorID) << " GPU: " << deviceProperties.deviceName << std::endl;

		bool hasValidExtensions = HasAllRequiredExtensions(vulkanPhysicalDevice);

		bool hasValidQueues = FindQueueFamilies(vulkanPhysicalDevice, surface).IsComplete();
		bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool hasValidSwapchain = false;

		if (hasValidExtensions)
		{
			VulkanSwapChainDetails swapChainDetails;
			swapChainDetails.Initialize(vulkanPhysicalDevice, surface);
			hasValidSwapchain = swapChainDetails.IsValid();
		}


		std::cout << "hasValidQueues: " << hasValidQueues << " hasValidExtensions: " << hasValidExtensions << " isDiscreteGPU: " << isDiscreteGPU << " hasValidSwapchain: " << hasValidSwapchain << std::endl;

		if (hasValidQueues && hasValidExtensions && isDiscreteGPU && hasValidSwapchain)
		{
			GraphicsContext::PhysicalDevice = vulkanPhysicalDevice;
		}
	}

	if (GraphicsContext::PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

QueueFamilyIndices GraphicsDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice, const InstanceWrapper<VkSurfaceKHR>& surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			indices.transportFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

bool GraphicsDevice::HasAllRequiredExtensions(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}
