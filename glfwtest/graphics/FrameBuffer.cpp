#include "FrameBuffer.h"
#include "graphics/GraphicsContext.h"

FrameBuffer::FrameBuffer() :
	semaphore( nullptr ),
	imageView( GraphicsContext::LogicalDevice, vkDestroyImageView, GraphicsContext::GlobalAllocator.Get() ),
	framebuffer( GraphicsContext::LogicalDevice, vkDestroyFramebuffer, GraphicsContext::GlobalAllocator.Get() )
{
}

FrameBuffer::~FrameBuffer()
{
	Destroy();
}

void FrameBuffer::Destroy()
{
	framebuffer = nullptr;
	imageView = nullptr;

	semaphore = nullptr;
}

void FrameBuffer::InitializeImageView( VkFormat imageFormat, VkImage image )
{
	format = imageFormat;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if ( vkCreateImageView( GraphicsContext::LogicalDevice, &viewInfo, imageView.AllocationCallbacks(), imageView.Replace() ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create texture image view!" );
	}
}

void FrameBuffer::InitializeFrameBuffer( uint32_t width, uint32_t height, DepthBuffer& depthBuffer )
{
	this->width = width;
	this->height = height;

	semaphore = std::make_shared<VulkanSemaphore>();

	std::array<VkImageView, 2> attachments = {
		GetImageView(),
		depthBuffer.GetImageView()
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = GraphicsContext::RenderPass->GetNative();
	framebufferInfo.attachmentCount = static_cast<uint32_t>( attachments.size() );
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	if ( vkCreateFramebuffer( GraphicsContext::LogicalDevice, &framebufferInfo, framebuffer.AllocationCallbacks(), framebuffer.Replace() ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create framebuffer!" );
	}
}
