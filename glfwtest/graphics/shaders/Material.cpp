#include "Material.h"
#include "io\FileSystem.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "graphics/buffers/UniformBuffer.h"

#include "graphics/textures/TextureSampler.h"
#include "graphics/textures/TextureLoader.h"

#include <thread>
#include <iostream>
#include <windows.h>

Material::Material(const std::string& fileName)
{
	std::cout << "Creating material: " << fileName << std::endl;
	//TODO: read the meta data and load in.
	//std::vector<char> data = FileSystem::ReadFile(fileName);

	vertex = ShaderCache::GetVertexShader(fileName);
	fragment = ShaderCache::GetFragmentShader(fileName);
	

	UniformBuffer* ub = new UniformBuffer((void*)(new CameraUBO()), sizeof(CameraUBO));
	uniformBuffers.push_back(ub);

	//VkFilter min, VkFilter mag, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode
	sampler = std::make_shared<TextureSampler>(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	texture = TextureLoader::Get("chalet.jpg");
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

	std::cout << "Destroyed material" << std::endl;
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
	return vertex->IsLoaded() && fragment->IsLoaded() && texture->IsLoaded();
}