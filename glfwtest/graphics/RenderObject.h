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

	void PrepareDraw()
	{
		
		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[i]->StartRecording(i);

			vkCmdBindPipeline(commandBuffers[i]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
			vkCmdDraw(commandBuffers[i]->GetNative(), 3, 1, 0, 0);

			commandBuffers[i]->StopRecording();
		}
	}

public:
	std::shared_ptr<Material> fixedMaterial;
	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
	PipelineStateObject pso;
};