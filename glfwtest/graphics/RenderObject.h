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
		standardMaterial->AddUniformBuffer(new UniformBuffer((void*)(new CameraUBO()), sizeof(CameraUBO)));

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
	

	std::shared_ptr<CommandBuffer> PrepareDraw(std::shared_ptr<CommandBuffer> commandBuffer, const Mesh& mesh)
	{
		if (!standardMaterial->IsLoaded())
			return nullptr;
		
		if (psoBasic3D.IsDirty())
		{
			psoBasic3D.Create(standardMaterial, std::vector<VkDynamicState>(), true);
			psoBasic3D.SetVertexLayout(mesh.GetBindingDescription(), mesh.GetAttributeDescriptions());
			psoBasic3D.Build();
		}

		standardMaterial->GetUniformBuffers()[0]->Upload();

		mesh.SetupCommandBuffer(commandBuffer, psoBasic3D, standardMaterial);

		return commandBuffer;
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