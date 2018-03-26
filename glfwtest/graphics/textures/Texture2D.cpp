#include "Texture2D.h"
#include "graphics/GraphicsContext.h"


Texture2D::Texture2D() :
	width(-1),
	height(-1),
	image(),
	imageView(),
	imageDeviceMemory(),
	format(VK_FORMAT_UNDEFINED)
{

}

Texture2D::~Texture2D()
{
	imageView = nullptr;
	image = nullptr;
	imageDeviceMemory = nullptr;
}


void Texture2D::SetImage(const VkImage& vkImage)
{
	image.Initialize(GraphicsContext::LogicalDevice, vkDestroyImage, GraphicsContext::GlobalAllocator.Get());

	image = vkImage;
}


void Texture2D::AllocateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlagBits imageUsage, VkMemoryPropertyFlagBits propertyFlags)
{
	this->width = width;
	this->height = height;

	image.Initialize(GraphicsContext::LogicalDevice, vkDestroyImage, GraphicsContext::GlobalAllocator.Get());
	imageDeviceMemory.Initialize(GraphicsContext::LogicalDevice, vkFreeMemory, GraphicsContext::GlobalAllocator.Get());

	GraphicsContext::DeviceAllocator->AllocateImage(width, height, format, imageTiling, imageUsage, propertyFlags, image, imageDeviceMemory);
}

void Texture2D::SetupView(VkFormat format, VkImageAspectFlags aspectFlags)
{
	this->format = format;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	imageView.Initialize(GraphicsContext::LogicalDevice, vkDestroyImageView, GraphicsContext::GlobalAllocator.Get());
		
	if (vkCreateImageView(GraphicsContext::LogicalDevice, &viewInfo, imageView.AllocationCallbacks(), imageView.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
}

//TODO: this function is in a bit of an odd place
void Texture2D::Transition(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	std::shared_ptr<CommandBuffer> commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	{

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (Vulkan::HasStencilComponent(format)) 
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else 
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; //You should pick the earliest pipeline stage that matches the specified operations, so that it is ready for usage as depth attachment when it needs to be.
		}
		else 
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(commandBuffer->GetNative(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	}


	commandBuffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->GetNative();

	vkQueueSubmit(GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsContext::TransportQueue);

	GraphicsContext::CommandBufferPoolTransient->Free(commandBuffer);
}
