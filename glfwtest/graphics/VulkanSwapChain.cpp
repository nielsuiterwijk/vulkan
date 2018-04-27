#include "VulkanSwapChain.h"

#include "GraphicsDevice.h"
#include "graphics\GraphicsContext.h"
#include "VulkanInstance.h"
#include "helpers\VulkanHelpers.h"

#include <algorithm>
#include "vec2.hpp"


VulkanSwapChain::VulkanSwapChain() :
	surface(GraphicsContext::VulkanInstance->GetNative(), vkDestroySurfaceKHR, GraphicsContext::GlobalAllocator.Get()),
	swapChain(),
	nextBackBufferIndex(0),
	depthBuffer()
{
}

VulkanSwapChain::~VulkanSwapChain()
{
	DestroyFrameBuffers();
	DestroySwapchain();
}

void VulkanSwapChain::Connect(const glm::u32vec2& windowSize, const QueueFamilyIndices& indices)
{
	details.Initialize(GraphicsContext::PhysicalDevice, surface);

	nextBackBufferIndex = 0;

	surfaceFormat = PickSwapSurfaceFormat(details.formats);
	presentMode = PickSwapPresentMode(details.presentModes, false);
	extent = GetSwapExtents(details.capabilities, windowSize);

	//Triple buffering!
	uint32_t imageCount = 3;

	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };


	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (indices.graphicsFamily == indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //always ignore alpha channel blending with other surfaces
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (swapChain == nullptr)
		createInfo.oldSwapchain = VK_NULL_HANDLE;
	else
		createInfo.oldSwapchain = *swapChain.Replace();

	swapChain.Initialize(GraphicsContext::LogicalDevice, vkDestroySwapchainKHR, GraphicsContext::GlobalAllocator.Get());

	if (vkCreateSwapchainKHR(GraphicsContext::LogicalDevice, &createInfo, swapChain.AllocationCallbacks(), swapChain.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	//The implementation is allowed to create more images, which is why we need to explicitly query the amount again.
	vkGetSwapchainImagesKHR(GraphicsContext::LogicalDevice, swapChain, &imageCount, nullptr); 
	
	std::vector<VkImage> images(imageCount);
	
	vkGetSwapchainImagesKHR(GraphicsContext::LogicalDevice, swapChain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;

	std::cout << "Created " << imageCount << " images of " << extent.width << "x" << extent.height << " format: " << Vulkan::GetFormatName(imageFormat) << std::endl;

	backBuffers.resize(imageCount);
	for (int i = 0; i < images.size(); i++)
	{
		backBuffers[i].InitializeImageView(imageFormat, images[i]);
	}

	depthBuffer.Initialize(windowSize.x, windowSize.y);
}

void VulkanSwapChain::SetupFrameBuffers()
{
	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		backBuffers[i].InitializeFrameBuffer(extent.width, extent.height, depthBuffer);
	}

	std::cout << "[Vulkan] created: " << backBuffers.size() << " back buffers." << std::endl;
}

void VulkanSwapChain::DestroyFrameBuffers()
{
	depthBuffer.Destroy();

	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		backBuffers[i].Destroy();
	}
}

void VulkanSwapChain::DestroySwapchain()
{
	swapChain = nullptr;
}

int32_t VulkanSwapChain::PrepareBackBuffer()
{	
	uint32_t imageIndex = -1;

	VkResult result = vkAcquireNextImageKHR(GraphicsContext::LogicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), backBuffers[nextBackBufferIndex].GetLock(), VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return -1;
	}
	
	assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	assert(imageIndex == nextBackBufferIndex); //imageIndex should be the same as backBufferIndex

	nextBackBufferIndex = (nextBackBufferIndex + 1) % backBuffers.size();

	return imageIndex;
}

VkSurfaceFormatKHR VulkanSwapChain::PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//Best case scenario, our surface accepts any format, lets pick the best combination
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//If we cannot choose any, lets see if there is a 'best' format available.
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	//If thats not possible, don't bother and use whatever.
	return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::PickSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync)
{
	//These are explained here: https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html
	if (waitForVSync)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
	}
	else
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				return availablePresentMode;
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::GetSwapExtents(const VkSurfaceCapabilitiesKHR& capabilities, const glm::u32vec2& windowSize)
{
	//The idea is to always pick the size of the window we render to, if that is not possible (told by have the current extent set to int32.max)
	//we'll try the largest size available.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D actualExtent = { windowSize.x, windowSize.y };

	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
	
}

InstanceWrapper<VkSurfaceKHR>& VulkanSwapChain::GetSurface()
{
	return surface;
}

const FrameBuffer& VulkanSwapChain::GetFrameBuffer(int32_t frameIndex)
{
	return backBuffers[frameIndex];
}

const VkSurfaceFormatKHR& VulkanSwapChain::GetSurfaceFormat() const
{
	return surfaceFormat;
}

const VkPresentModeKHR& VulkanSwapChain::GetPresentMode() const
{
	return presentMode;
}

VkExtent2D VulkanSwapChain::GetExtent() const
{
	return extent;
}

const DepthBuffer& VulkanSwapChain::GetDepthBuffer() const
{
	return depthBuffer;
}

const InstanceWrapper<VkSwapchainKHR>& VulkanSwapChain::GetNative() const
{
	return swapChain;
}

void VulkanSwapChainDetails::Initialize(const VkPhysicalDevice& physicalDevice, const InstanceWrapper<VkSurfaceKHR>& surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	//TODO: See if there is a way to make these sort of calls a helper method?
	{
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
		}
	}

	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
		}
	}
}

bool VulkanSwapChainDetails::IsValid() const
{
	return !formats.empty() && !presentModes.empty();
}
