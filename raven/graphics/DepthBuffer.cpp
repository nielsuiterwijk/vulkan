#include "DepthBuffer.h"

#include "graphics/GraphicsContext.h"

DepthBuffer::DepthBuffer()
	: Texture2D()
{
}

DepthBuffer::~DepthBuffer()
{
	//Destroy() gets called in Texture2D deconstructor
}

void DepthBuffer::Destroy()
{
	//(destroy) Order: view -> image -> memory
	_View = nullptr;

	vmaDestroyImage( *GraphicsContext::DeviceAllocator, _Resource.Image,
					 _Resource.Allocation ); //TODO: Maybe also have this go through the GPUAllocator class?

	_Resource.Image = nullptr;
	_Resource.Allocation = nullptr;
}

void DepthBuffer::Initialize( uint32_t width, uint32_t height )
{
	VkFormat depthFormat = FindDepthFormat();
	AllocateImage( width, height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY );
	SetupView( depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );
	Transition( depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
}

VkFormat DepthBuffer::FindSupportedFormat( const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
{
	for ( VkFormat format : candidates )
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties( GraphicsContext::PhysicalDevice, format, &props );

		if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
		{
			return format;
		}
		else if ( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
		{
			return format;
		}
	}

	ASSERT_FAIL( "failed to find supported format!" );

	return VK_FORMAT_UNDEFINED;
}

VkFormat DepthBuffer::FindDepthFormat()
{
	return FindSupportedFormat( { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}
