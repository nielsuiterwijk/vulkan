#pragma once

#include "helpers\InstanceWrapper.h"

class GraphicsDevice;

class RenderPass
{
public:
	RenderPass( VkFormat frameBufferFormat, VkFormat depthFormat );
	~RenderPass();

	const InstanceWrapper<VkRenderPass>& GetNative() const;

	uint32_t GetHash() const { return _Hash; }
private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentDescription depthAttachment;

	VkAttachmentReference colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef;

	VkSubpassDescription subpass;

	InstanceWrapper<VkRenderPass> renderPass;
	uint32_t _Hash;
};
