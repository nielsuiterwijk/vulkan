#pragma once

#include "PipelineStateObject.h"
#include "graphics/models/Mesh.h"
#include "graphics/VulkanSwapChain.h"
#include "graphics/buffers/UniformBuffer.h"
class RenderObject
{
public:
	RenderObject()
	{

	}

	~RenderObject()
	{
		//fixedMaterial = nullptr;
		//material2D = nullptr;
		//material3D = nullptr;
		standardMaterial = nullptr;
	}

	void Load(const Mesh& m)
	{
		/*fixedMaterial = GraphicsDevice::Instance().CreateMaterial("fixed");
		material2D = GraphicsDevice::Instance().CreateMaterial("basic2d");
		material3D = GraphicsDevice::Instance().CreateMaterial("basic3d");*/
		standardMaterial = GraphicsDevice::Instance().CreateMaterial("standard");

		/*psoFixed.Create(fixedMaterial);
		psoFixed.Build();

		psoBasic2D.Create(material2D);
		psoBasic2D.SetVertices(m.GetBindingDescription(), m.GetAttributeDescriptions());
		psoBasic2D.Build();

		psoBasic3D.Create(material3D);
		psoBasic3D.SetVertices(m.GetBindingDescription(), m.GetAttributeDescriptions());
		psoBasic3D.Build();*/

		GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);
	}

	std::shared_ptr<CommandBuffer> ClearBackbuffer(uint32_t imageIndex)
	{
		if (commandBuffers.size() == 0)
		{
			GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);
		}

		std::shared_ptr<CommandBuffer> commandBuffer = commandBuffers[imageIndex];

		commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
		renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).GetNative();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
		clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 means pixel is furthest away, so stuff can be rendered on top of it.

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(commandBuffer->GetNative());
		commandBuffer->StopRecording();

		return commandBuffer;
	}


	std::shared_ptr<CommandBuffer> PrepareDraw(uint32_t imageIndex, const Mesh& mesh)
	{
		if (!standardMaterial->IsLoaded())
			return nullptr;

		std::shared_ptr<CommandBuffer> commandBuffer = commandBuffers[imageIndex];

		if (psoBasic3D.IsDirty())
		{
			psoBasic3D.Create(standardMaterial);
			psoBasic3D.SetVertices(mesh.GetBindingDescription(), mesh.GetAttributeDescriptions());
			psoBasic3D.Build();
		}

		standardMaterial->GetUniformBuffers()[0]->Upload();
		
		{
			commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
			renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).GetNative();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
			clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 means pixel is furthest away, so stuff can be rendered on top of it.

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				//mesh.SetupCommandBuffer(commandBuffer, psoBasic2D, material2D);
				mesh.SetupCommandBuffer(commandBuffer, psoBasic3D, standardMaterial);
			}
			vkCmdEndRenderPass(commandBuffer->GetNative());
			commandBuffer->StopRecording();
		}

		return commandBuffer;
	}

	void PrepareDraw(uint32_t imageIndex)
	{
		
		//for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[imageIndex]->StartRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = GraphicsContext::RenderPass->GetRenderPass();
			renderPassInfo.framebuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).GetNative();
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
	//std::shared_ptr<Material> fixedMaterial;
	//std::shared_ptr<Material> material2D;
	//std::shared_ptr<Material> material3D;
	std::shared_ptr<Material> standardMaterial;

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	PipelineStateObject psoFixed;
	PipelineStateObject psoBasic2D;
	PipelineStateObject psoBasic3D;
};