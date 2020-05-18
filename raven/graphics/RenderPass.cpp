#include "RenderPass.h"

#include <vulkan\vulkan.h>

#include "graphics\GraphicsContext.h"
#include "graphics\GraphicsDevice.h"
#include "graphics\VulkanSwapChain.h"
#include "helpers/Murmur3.h"

RenderPass::RenderPass( VkFormat frameBufferFormat, VkFormat depthFormat )
	: renderPass( GraphicsContext::LogicalDevice, vkDestroyRenderPass, GraphicsContext::GlobalAllocator.Get() )
	, subpass()
	, colorAttachment()
	, colorAttachmentRef()
	, depthAttachment()
	, depthAttachmentRef()
	, _Hash( 0 )
{
	colorAttachment.format = frameBufferFormat;
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

	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.flags = 0;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>( attachments.size() );
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ( vkCreateRenderPass( GraphicsContext::LogicalDevice, &renderPassInfo, renderPass.AllocationCallbacks(), renderPass.Replace() ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to create render pass!" );
	}

	VkRenderPass tmp = renderPass;
	size_t a = sizeof( tmp );
	size_t b = sizeof( VkRenderPass );

	_Hash = Murmur3::Hash( &colorAttachment, sizeof( VkAttachmentDescription ), _Hash );
	_Hash = Murmur3::Hash( &depthAttachment, sizeof( VkAttachmentDescription ), _Hash );
	_Hash = Murmur3::Hash( &dependency, sizeof( VkSubpassDependency ), _Hash );
	_Hash = Murmur3::Hash( reinterpret_cast<uint64_t>(tmp), _Hash ); //Hash the existing pointer into it.
}

RenderPass::~RenderPass()
{
	renderPass = nullptr;
	std::cout << "Destroyed renderpass" << std::endl;
}

const InstanceWrapper<VkRenderPass>& RenderPass::GetNative() const
{
	return renderPass;
}
