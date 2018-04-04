#include "Material.h"
#include "io\FileSystem.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "graphics/buffers/UniformBuffer.h"

#include <thread>
#include <iostream>
#include <windows.h>

Material::Material(const std::string& fileName) :
	descripterPool(1)
{
	std::cout << "Creating material: " << fileName << std::endl;
	//TODO: read the meta data and load in.
	//std::vector<char> data = FileSystem::ReadFile(fileName);

	vertex = ShaderCache::GetVertexShader(fileName);
	fragment = ShaderCache::GetFragmentShader(fileName);

	::Sleep(100);

	shaderStages.push_back(vertex->GetShaderStageCreateInfo());
	shaderStages.push_back(fragment->GetShaderStageCreateInfo());

	UniformBuffer* ub = new UniformBuffer((void*)(new CameraUBO()), sizeof(CameraUBO), descripterPool);
	uniformBuffers.push_back(ub);
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

const std::vector<VkPipelineShaderStageCreateInfo>& Material::GetShaderStages() const
{
	return shaderStages;
}

const std::vector<UniformBuffer*>& Material::GetUniformBuffers() const
{
	return uniformBuffers;
}
