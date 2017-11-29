#include "Material.h"
#include "io\FileSystem.h"
#include "ShaderCache.h"
#include "VertexShader.h"
#include "FragmentShader.h"

Material::Material(const std::string& fileName)
{
	//TODO: read the meta data and load in.
	//std::vector<char> data = FileSystem::ReadFile(fileName);

	vertex = ShaderCache::GetVertexShader(fileName);
	fragment = ShaderCache::GetFragmentShader(fileName);

	shaderStages.push_back(vertex->GetShaderStageCreateInfo());
	shaderStages.push_back(fragment->GetShaderStageCreateInfo());
}

Material::~Material()
{
	vertex = nullptr;
	fragment = nullptr;

	std::cout << "Destroyed material" << std::endl;
}

const std::vector<VkPipelineShaderStageCreateInfo>& Material::GetShaderStages() const
{
	return shaderStages;
}
