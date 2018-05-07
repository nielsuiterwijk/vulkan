#include "Material.h"
#include "io\FileSystem.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "graphics/buffers/UniformBuffer.h"

#include "graphics/textures/TextureSampler.h"
#include "graphics/textures/TextureLoader.h"

#include "jsonxx/jsonxx.h"

#include <thread>
#include <iostream>
#include <windows.h>

Material::Material(const std::string& fileName) :
	vertex(nullptr),
	fragment(nullptr),
	texture(nullptr),
	sampler(nullptr)
{
	std::cout << "Creating material: " << fileName << std::endl;	
	FileSystem::LoadFileAsync("materials/" + fileName + ".mat", std::bind(&Material::FileLoaded, this, std::placeholders::_1));
}

Material::~Material()
{
	vertex = nullptr;
	fragment = nullptr;

	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		delete uniformBuffers[i];
	}
	uniformBuffers.clear();

	texture = nullptr;
	sampler = nullptr;

	std::cout << "Destroyed material" << std::endl;
}

void Material::FileLoaded(std::vector<char> fileData)
{
	std::string fileContents(fileData.data(), fileData.size());
	jsonxx::Object jsonObject;
	bool result = jsonObject.parse(fileContents);
	assert(result);

	vertex = ShaderCache::GetVertexShader(jsonObject.get<std::string>("shader"));
	fragment = ShaderCache::GetFragmentShader(jsonObject.get<std::string>("shader"));

	//										   VkFilter min,	VkFilter mag,	VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode
	sampler = std::make_shared<TextureSampler>(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	if (jsonObject.has<std::string>("texture"))
	{
		texture = TextureLoader::Get(jsonObject.get<std::string>("texture"));
	}
}

void Material::AddUniformBuffer(UniformBuffer* uniformBuffer)
{
	uniformBuffers.push_back(uniformBuffer);
}

const std::vector<VkPipelineShaderStageCreateInfo>& Material::GetShaderStages()
{
	//TODO: Not the right place, should be done on a callback of some sorts.
	if (IsLoaded() && shaderStages.size() == 0)
	{
		std::cout << "Initializing shader stages" << std::endl;
		shaderStages.push_back(vertex->GetShaderStageCreateInfo());
		shaderStages.push_back(fragment->GetShaderStageCreateInfo());
	}

	return shaderStages;
}

const std::vector<UniformBuffer*>& Material::GetUniformBuffers() const
{
	return uniformBuffers;
}

bool Material::IsLoaded() const 
{ 
	if (vertex == nullptr || fragment == nullptr)
	{
		return false;
	}

	return vertex->IsLoaded() && fragment->IsLoaded() && (texture == nullptr || texture->IsLoaded());
}