#include "VulkanSwapChain.h"

#include "GraphicsDevice.h"
#include "VulkanInstance.h"
#include "helpers\VulkanHelpers.h"

#include <algorithm>
#include "vec2.hpp"


VulkanSwapChain::VulkanSwapChain(const InstanceWrapper<VkInstance>& applicationInfo) :
	surface(applicationInfo, vkDestroySurfaceKHR)
{

}

VulkanSwapChain::~VulkanSwapChain()
{
	for (int i = 0; i < imageViews.size(); i++)
	{
		imageViews[i] = nullptr;
	}

	imageViews.clear();

	swapChain = nullptr;
	surface = nullptr;
}


void VulkanSwapChain::Connect(const glm::u32vec2& windowSize, const QueueFamilyIndices& indices)
{
	details.Initialize(GraphicsContext::PhysicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = GetSwapPresentMode(details.presentModes, false);
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
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	swapChain = InstanceWrapper<VkSwapchainKHR>(GraphicsContext::LogicalDevice, vkDestroySwapchainKHR);

	if (vkCreateSwapchainKHR(GraphicsContext::LogicalDevice, &createInfo, swapChain.AllocationCallbacks(), swapChain.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(GraphicsContext::LogicalDevice, swapChain, &imageCount, nullptr); //The implementation is allowed to create more images, which is why we need to explicitly query the amount again.
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(GraphicsContext::LogicalDevice, swapChain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;

	std::cout << "Created " << imageCount << " images of " << extent.width << " x " << extent.height << " format: " << Vulkan::GetFormatName(imageFormat) << std::endl;

	imageViews.resize(images.size());

	for (int i = 0; i < images.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		//This is a typical view setup (for a frame buffer). No mipmaps, just color
		imageViews[i] = InstanceWrapper<VkImageView> { GraphicsContext::LogicalDevice, vkDestroyImageView, true };

		if (vkCreateImageView(GraphicsContext::LogicalDevice, &createInfo, imageViews[i].AllocationCallbacks(), imageViews[i].Replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}


	}

	int asd = 0;
}


VkSurfaceFormatKHR VulkanSwapChain::GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR VulkanSwapChain::GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool waitForVSync)
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
	else
	{
		VkExtent2D actualExtent = { windowSize.x, windowSize.y };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

InstanceWrapper<VkSurfaceKHR>& VulkanSwapChain::GetSurface()
{
	return surface;
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