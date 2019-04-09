#include "model.h"

#include "io/FileSystem.h"

#include "graphics/models/MeshFileLoader.h"
#include "graphics/textures/TextureLoader.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/shaders/Material.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSampler.h"
#include "graphics/models/SkinnedMesh.h"
#include "graphics/models/SubMesh.h"
#include "graphics/shaders/VertexShader.h"


Model::Model(const std::string& objectFile) : 
	material(nullptr),
	mesh(nullptr)
{
	FileSystem::LoadFileAsync("gameobjects/" + objectFile + ".gameobject", std::bind(&Model::FileLoaded, this, std::placeholders::_1));
}

Model::~Model()
{

}

void Model::FileLoaded(std::vector<char> fileData)
{
	std::string fileContents(fileData.data(), fileData.size());

	auto jsonObject = json::parse(fileContents);
	
	std::string meshFileName = jsonObject["mesh"];
	std::string materialFileName = jsonObject["material"];

	mesh = std::static_pointer_cast<SkinnedMesh>(MeshFileLoader::Skinned(meshFileName));
	material = std::make_shared<Material>(materialFileName);
	material->AddUniformBuffer(new UniformBuffer( { static_cast<void*>(&camera), sizeof(CameraUBO) } ));
	
	std::vector<std::string> texturesJson = jsonObject["textures"];

	std::vector<std::string> textureFileNames;

	for (std::string textureFileName : texturesJson)
	{
		bool alreadyAdded = false;

		for (int j = 0; j < textureFileNames.size(); j++)
		{
			if (textureFileNames[j] == textureFileName)
			{
				textureFileNames.push_back(textureFileName);
				textures.push_back(textures[j]);
				alreadyAdded = true;
				break;
			}
		}

		if (!alreadyAdded)
		{
			textureFileNames.push_back(textureFileName);
			textures.push_back(TextureLoader::Get(textureFileName));
		}
	}


}

void Model::WindowResized(int w, int h)
{
	if (!pso.IsDirty())
	{
		pso.SetViewPort(w, h);
		pso.Build();
	}
}

bool Model::TexturesLoaded() const
{
	for (int i = 0; i < textures.size(); i++)
	{
		if (!textures[i]->IsLoaded())
			return false;
	}

	return true;
}


void Model::Draw(std::shared_ptr<CommandBuffer> commandBuffer)
{
	if (material == nullptr)
		return;

	if (!material->IsLoaded() || !mesh->IsLoaded() || !TexturesLoaded())
		return;

	mesh->Update(0.001f);

	if (material->GetUniformBuffers().size() != 2)
	{
		material->AddUniformBuffer(mesh->AccessUBO());
	}
	
	material->UpdateUniformBuffers();
	

	if (pso.IsDirty())
	{
		mesh->BuildDescriptors(material);

		material->GetSampler()->Initialize(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, textures[0]->GetMipLevels());
		pso.Create(material, std::vector<VkDynamicState>(), true);
		pso.SetVertexLayout(mesh->GetBindingDescription(), mesh->GetAttributeDescriptions());
		pso.Build();
	}
	
	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
	

	const std::vector<SubMesh*>& meshes = mesh->GetSubMeshes();
	for (int i = 0; i < meshes.size(); i++)
	{
		VkDescriptorSet set = GraphicsContext::DescriptorPool->GetDescriptorSet(material, textures[i].get(), material->GetSampler().get());
		vkCmdBindDescriptorSets(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsContext::PipelineLayout, 0, 1, &set, 0, nullptr);

		meshes[i]->Draw(commandBuffer);
	}
}