#include "model.h"

#include "io/FileSystem.h"
#include "jsonxx/jsonxx.h"

#include "graphics/models/MeshFileLoader.h"
#include "graphics/textures/TextureLoader.h"

#include "graphics/buffers/UniformBuffer.h"
#include "graphics/shaders/Material.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSampler.h"
#include "graphics/models/Mesh.h"
#include "graphics/models/SubMesh.h"


Model::Model(const std::string& objectFile) : 
	material(nullptr),
	camera(nullptr),
	mesh(nullptr)
{
	FileSystem::LoadFileAsync("gameobjects/" + objectFile + ".gameobject", std::bind(&Model::FileLoaded, this, std::placeholders::_1));

	camera = std::make_shared<CameraUBO>();
}

Model::~Model()
{

}

void Model::FileLoaded(std::vector<char> fileData)
{
	std::string fileContents(fileData.data(), fileData.size());
	jsonxx::Object jsonObject;
	bool result = jsonObject.parse(fileContents);
	assert(result);

	std::string meshFileName = jsonObject.get<std::string>("mesh");
	std::string materialFileName = jsonObject.get<std::string>("material");

	mesh = MeshFileLoader::Get(meshFileName);
	material = std::make_shared<Material>(materialFileName);
	material->AddUniformBuffer(new UniformBuffer(camera, sizeof(CameraUBO)));
	
	jsonxx::Array texturesJson = jsonObject.get<jsonxx::Array>("textures");

	std::vector<std::string> textureFileNames;

	for (int i = 0; i < texturesJson.size(); i++)
	{
		std::string textureFileName = texturesJson.get<std::string>(i);

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

	if (pso.IsDirty())
	{
		material->GetSampler()->Initialize(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, textures[0]->GetMipLevels());
		pso.Create(material, std::vector<VkDynamicState>(), true);
		pso.SetVertexLayout(mesh->GetBindingDescription(), mesh->GetAttributeDescriptions());
		pso.Build();
	}

	material->GetUniformBuffers()[0]->Upload();
	
	const std::vector<SubMesh*>& meshes = mesh->GetSubMeshes();

	vkCmdBindPipeline(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
	

	for (int i = 0; i < meshes.size(); i++)
	{
		VkDescriptorSet set = GraphicsContext::DescriptorPool->GetDescriptorSet(material->GetUniformBuffers()[0], textures[i].get(), material->GetSampler().get());
		vkCmdBindDescriptorSets(commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsContext::PipelineLayout, 0, 1, &set, 0, nullptr);

		meshes[i]->Draw(commandBuffer);
	}
}

std::shared_ptr<CameraUBO> Model::GetUBO() const
{
	return camera;
}
