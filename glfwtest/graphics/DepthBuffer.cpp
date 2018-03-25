#include "DepthBuffer.h"

#include "graphics/GraphicsContext.h"

DepthBuffer::DepthBuffer() : Texture2D()
{
	
}

DepthBuffer::~DepthBuffer()
{

}

void DepthBuffer::Initialize(uint32_t width, uint32_t height)
{
	VkFormat depthFormat = FindDepthFormat();
	AllocateImage(width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	SetupView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	Transition(depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

void DepthBuffer::Destroy()
{
	imageView = nullptr;
	image = nullptr;
	imageDeviceMemory = nullptr;
}

VkFormat DepthBuffer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(GraphicsContext::PhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");	
}

VkFormat DepthBuffer::FindDepthFormat()
{
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}