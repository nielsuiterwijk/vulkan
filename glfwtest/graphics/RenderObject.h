#pragma once

#include "RavenApp.h"

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
		standardMaterial = nullptr;
		//todo: remove
		//RavenApp::OnWindowResized.push_back(std::bind(&RenderObject::WindowResized, this, _1, _2));
	}

	void Load(const Mesh& m)
	{
		camera = std::make_shared<CameraUBO>();

		/*fixedMaterial = GraphicsDevice::Instance().CreateMaterial("fixed");
		material2D = GraphicsDevice::Instance().CreateMaterial("basic2d");
		material3D = GraphicsDevice::Instance().CreateMaterial("basic3d");*/
		standardMaterial = GraphicsDevice::Instance().CreateMaterial("standard");
		standardMaterial->AddUniformBuffer(new UniformBuffer(camera, sizeof(CameraUBO)));

		/*psoFixed.Create(fixedMaterial);
		psoFixed.Build();

		psoBasic2D.Create(material2D);
		psoBasic2D.SetVertices(m.GetBindingDescription(), m.GetAttributeDescriptions());
		psoBasic2D.Build();

		psoBasic3D.Create(material3D);
		psoBasic3D.SetVertices(m.GetBindingDescription(), m.GetAttributeDescriptions());
		psoBasic3D.Build();*/

		using namespace std::placeholders;

		//int a = RavenApp::OnWindowResized.size();
		//RavenApp::OnWindowResized.push_back(std::bind(&RenderObject::WindowResized, this, _1, _2));

	}

	void WindowResized(int w, int h)
	{
		if (!psoBasic3D.IsDirty())
		{
			psoBasic3D.SetViewPort(w, h);
			psoBasic3D.Build();
		}
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
	std::shared_ptr<Material> standardMaterial;

	std::shared_ptr<CameraUBO> camera;
	
	PipelineStateObject psoBasic3D;
};