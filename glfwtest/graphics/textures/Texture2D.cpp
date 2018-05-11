#include "Texture2D.h"
#include "graphics/GraphicsContext.h"


Texture2D::Texture2D() :
	width(-1),
	height(-1),
	mipLevels(-1),
	image(),
	imageView(),
	imageDeviceMemory(),
	format(VK_FORMAT_UNDEFINED)
{

}

Texture2D::~Texture2D()
{
	//Destroy view -> image -> memory
	imageView = nullptr;
	image = nullptr;
	imageDeviceMemory = nullptr;
}


void Texture2D::SetImage(const VkImage& vkImage)
{
	image.Initialize(GraphicsContext::LogicalDevice, vkDestroyImage, GraphicsContext::GlobalAllocator.Get());

	image = vkImage;
}


void Texture2D::AllocateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlagBits imageUsage, VkMemoryPropertyFlagBits propertyFlags)
{
	assert(width < 16384);
	assert(height < 16384);
	assert(mipLevels < 14);

	this->width = width;
	this->height = height;
	this->mipLevels = mipLevels;

	if (mipLevels > 1)
	{
		imageUsage = (VkImageUsageFlagBits)(((int)imageUsage) | (int)VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	}

	image.Initialize(GraphicsContext::LogicalDevice, vkDestroyImage, GraphicsContext::GlobalAllocator.Get());
	imageDeviceMemory.Initialize(GraphicsContext::LogicalDevice, vkFreeMemory, GraphicsContext::GlobalAllocator.Get());

	GraphicsContext::DeviceAllocator->AllocateImage(width, height, mipLevels, format, imageTiling, imageUsage, propertyFlags, image, imageDeviceMemory);
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
	viewInfo.subresourceRange.levelCount = mipLevels;
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
		barrier.subresourceRange.levelCount = mipLevels;
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

	GraphicsContext::QueueLock.lock();
	VkResult result = vkQueueSubmit(GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(result == VK_SUCCESS);
	result = vkQueueWaitIdle(GraphicsContext::TransportQueue);
	assert(result == VK_SUCCESS);
	GraphicsContext::QueueLock.unlock();


	GraphicsContext::CommandBufferPoolTransient->Free(commandBuffer);
}


void Texture2D::GenerateMipMaps()
{
	std::shared_ptr<CommandBuffer> commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	//Note: starts at 1, not 0!
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->GetNative(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		//blit from larger source to smaller destination
		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth / 2, mipHeight / 2, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		
		//The source mip level was just transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL (using the memory barrier)
		//and the destination level is still in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL from the initial Transition call.
		//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdBlitImage.html
		vkCmdBlitImage(commandBuffer->GetNative(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		//Now transition back from SRC_OPTIMAL to SHADER_READ_ONLY_OPTIMAL. This waits on the previous command to be done 
		//as the previous memory barrier waits until every command in the TRANSFER stage is done.
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->GetNative(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)
		{
			mipWidth /= 2;
		}

		if (mipHeight > 1)
		{
			mipHeight /= 2;
		}
	}

	//Final, as the last source is never blitted from, thus never transitioned 
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer->GetNative(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,	1, &barrier);

	commandBuffer->StopRecording();

	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->GetNative();

		GraphicsContext::QueueLock.lock();
		VkResult result = vkQueueSubmit(GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);
		result = vkQueueWaitIdle(GraphicsContext::TransportQueue);
		GraphicsContext::QueueLock.unlock();
		assert(result == VK_SUCCESS);

		GraphicsContext::CommandBufferPoolTransient->Free(commandBuffer);
	}
}

bool Texture2D::IsLoaded() const
{
	return imageView != nullptr;
}