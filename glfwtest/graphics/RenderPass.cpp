#include "RenderPass.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\VulkanSwapChain.h"

#include <vulkan\vulkan.h>

RenderPass::RenderPass(GraphicsDevice* device) :
	renderPass(GraphicsContext::LogicalDevice, vkDestroyRenderPass),
	subpass(),
	colorAttachment(),
	colorAttachmentRef()
{
	colorAttachment.format = device->GetSwapChain()->GetSurfaceFormat().format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.flags = 0;

	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentRef.attachment = 0;

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.flags = 0;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(GraphicsContext::LogicalDevice, &renderPassInfo, renderPass.AllocationCallbacks(), renderPass.Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

RenderPass::~RenderPass()
{
	renderPass = nullptr;
	std::cout << "Destroyed renderpass" << std::endl;
}

const InstanceWrapper<VkRenderPass>& RenderPass::GetRenderPass() const
{
	return renderPass;
}