#pragma once

#include "textures/Texture2D.h"
#include "graphics/helpers/VulkanSemaphore.h"
#include "graphics/DepthBuffer.h"

//A specialized Texture2D class, to specialized for inheritance
class FrameBuffer
{
public:
	FrameBuffer();
	virtual ~FrameBuffer();

	void Destroy();

	void InitializeImageView(VkFormat imageFormat, VkImage image);
	void InitializeFrameBuffer(uint32_t width, uint32_t height, DepthBuffer& depthBuffer);

	const InstanceWrapper<VkFramebuffer>& GetNative() const { return framebuffer; }
	const InstanceWrapper<VkSemaphore>& GetLock() const { return semaphore->GetNative(); }

	const InstanceWrapper<VkImageView>& GetImageView() const { return imageView; }

private:

	InstanceWrapper<VkImageView> imageView;

	InstanceWrapper<VkFramebuffer> framebuffer;
	std::shared_ptr<VulkanSemaphore> semaphore;

	uint32_t width;
	uint32_t height;

	VkFormat format;
};