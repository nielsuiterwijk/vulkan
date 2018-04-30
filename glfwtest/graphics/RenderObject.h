#pragma once

#include "RavenApp.h"

#include "PipelineStateObject.h"
#include "graphics/models/Mesh.h"
#include "graphics/VulkanSwapChain.h"
#include "graphics/buffers/UniformBuffer.h"

#include "graphics/models/MeshFileLoader.h"

class RenderObject
{
public:
	RenderObject() :
		mesh(nullptr)
	{

	}

	~RenderObject()
	{
		standardMaterial = nullptr;
		camera = nullptr;
		mesh = nullptr;
	}

	void Load()
	{
		camera = std::make_shared<CameraUBO>();

		//standardMaterial = GraphicsDevice::Instance().CreateMaterial("chalet.mat");
		//mesh = MeshFileLoader::Get("chalet.obj");

		//standardMaterial = GraphicsDevice::Instance().CreateMaterial("cube.mat");
		//mesh = MeshFileLoader::Get("cube.obj");

		standardMaterial = GraphicsDevice::Instance().CreateMaterial("cube.mat");
		mesh = MeshFileLoader::Get("Ral Zarek.stl");


		standardMaterial->AddUniformBuffer(new UniformBuffer(camera, sizeof(CameraUBO)));
		
	}

	void WindowResized(int w, int h)
	{
		if (!psoBasic3D.IsDirty())
		{
			psoBasic3D.SetViewPort(w, h);
			psoBasic3D.Build();
		}
	}
	

	void PrepareDraw(std::shared_ptr<CommandBuffer> commandBuffer)
	{
		if (!standardMaterial->IsLoaded() || !mesh->IsLoaded())
			return;
		
		if (psoBasic3D.IsDirty())
		{
			psoBasic3D.Create(standardMaterial, std::vector<VkDynamicState>(), true);
			psoBasic3D.SetVertexLayout(mesh->GetBindingDescription(), mesh->GetAttributeDescriptions());
			psoBasic3D.Build();
		}

		standardMaterial->GetUniformBuffers()[0]->Upload();

		mesh->SetupCommandBuffer(commandBuffer, psoBasic3D, standardMaterial);
	}

public:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> standardMaterial;
	std::shared_ptr<CameraUBO> camera;
	
	PipelineStateObject psoBasic3D;
};