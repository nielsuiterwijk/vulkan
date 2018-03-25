#pragma once

#include "helpers\InstanceWrapper.h"

#include "standard.h"

class GraphicsDevice;

class RenderPass
{
public:
	RenderPass(VkFormat frameBufferFormat, VkFormat depthFormat);
	~RenderPass();

	const InstanceWrapper<VkRenderPass>& GetRenderPass() const;

private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentDescription depthAttachment;

	VkAttachmentReference colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef;

	VkSubpassDescription subpass;

	InstanceWrapper<VkRenderPass> renderPass;
};