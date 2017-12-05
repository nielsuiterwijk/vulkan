#pragma once

#include "PipelineStateObject.h"
#include "graphics/models/Mesh.h"

class RenderObject
{
public:
	RenderObject()
	{

	}

	~RenderObject()
	{
		fixedMaterial = nullptr;
		basicMaterial = nullptr;
	}

	void Load(const Mesh& m)
	{
		fixedMaterial = GraphicsDevice::Instance().CreateMaterial("fixed");
		basicMaterial = GraphicsDevice::Instance().CreateMaterial("basic");

		psoFixed.Create(fixedMaterial);
		psoFixed.Build();

		psoBasic.Create(nullptr);

		psoBasic.SetShader(basicMaterial->GetShaderStages());
		psoBasic.SetVertices(m.GetBindingDescription(), m.GetAttributeDescriptions());
		psoBasic.Build();

		GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);
	}

	void PrepareDraw(uint32_t imageIndex, const Mesh& mesh)
	{

		//for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[imageIndex]->StartRecording(imageIndex, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);


			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
			renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

			VkClearValue clearColor = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[imageIndex]->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				mesh.SetupCommandBuffer(commandBuffers[imageIndex], psoBasic);
			}
			vkCmdEndRenderPass(commandBuffers[imageIndex]->GetNative());
			commandBuffers[imageIndex]->StopRecording();
		}
	}

	void PrepareDraw(uint32_t imageIndex)
	{
		
		//for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[imageIndex]->StartRecording(imageIndex, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
			renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

			VkClearValue clearColor = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[imageIndex]->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(commandBuffers[imageIndex]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, psoFixed.GetPipeLine());
				vkCmdDraw(commandBuffers[imageIndex]->GetNative(), 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(commandBuffers[imageIndex]->GetNative());

			commandBuffers[imageIndex]->StopRecording();
		}
	}

public:
	std::shared_ptr<Material> fixedMaterial;
	std::shared_ptr<Material> basicMaterial;

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	PipelineStateObject psoFixed;
	PipelineStateObject psoBasic;
};