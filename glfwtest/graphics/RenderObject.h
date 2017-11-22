#pragma once

#include "PipelineStateObject.h"

class RenderObject
{
public:
	RenderObject()
	{

	}

	~RenderObject()
	{
		fixedMaterial = nullptr;
	}

	void Load()
	{
		fixedMaterial = GraphicsDevice::Instance().CreateMaterial("fixed");
		pso.Create(fixedMaterial);


		GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);
	}

	void PrepareDraw(uint32_t imageIndex)
	{
		
		//for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[imageIndex]->StartRecording(imageIndex);

			vkCmdBindPipeline(commandBuffers[imageIndex]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
			vkCmdDraw(commandBuffers[imageIndex]->GetNative(), 3, 1, 0, 0);

			commandBuffers[imageIndex]->StopRecording();
		}
	}

public:
	std::shared_ptr<Material> fixedMaterial;
	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
	PipelineStateObject pso;
};