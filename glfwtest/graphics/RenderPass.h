#pragma once

#include "helpers\InstanceWrapper.h"

#include "standard.h"

class GraphicsDevice;

class RenderPass
{
public:
	RenderPass(VkFormat format);
	~RenderPass();

	const InstanceWrapper<VkRenderPass>& GetRenderPass() const;

private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference colorAttachmentRef;
	VkSubpassDescription subpass;

	InstanceWrapper<VkRenderPass> renderPass;
};