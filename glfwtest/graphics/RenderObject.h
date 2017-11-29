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
			commandBuffers[imageIndex]->StartRecording(imageIndex);

			mesh.SetupCommandBuffer(commandBuffers[imageIndex], psoBasic);

			commandBuffers[imageIndex]->StopRecording();
		}
	}

	void PrepareDraw(uint32_t imageIndex)
	{
		
		//for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[imageIndex]->StartRecording(imageIndex);

			vkCmdBindPipeline(commandBuffers[imageIndex]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, psoFixed.GetPipeLine());
			vkCmdDraw(commandBuffers[imageIndex]->GetNative(), 3, 1, 0, 0);

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