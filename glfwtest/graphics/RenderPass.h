#pragma once

#include "helpers\InstanceWrapper.h"

class GraphicsDevice;

class RenderPass
{
public:
	RenderPass(GraphicsDevice* device);
	~RenderPass();

	const InstanceWrapper<VkRenderPass>& GetRenderPass() const;

private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference colorAttachmentRef;
	VkSubpassDescription subpass;

	InstanceWrapper<VkRenderPass> renderPass;
};