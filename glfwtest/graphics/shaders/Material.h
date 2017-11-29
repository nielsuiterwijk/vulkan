#pragma once

#include "graphics\helpers\InstanceWrapper.h"

#include <string>
#include <vector>
#include <memory>

class VertexShader;
class FragmentShader;

//.material file, which is a meta file to the textures and shaders needed.
class Material
{
public:
	Material(const std::string& fileName);
	~Material();

	const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const;

private:
	std::shared_ptr<VertexShader> vertex;
	std::shared_ptr<FragmentShader> fragment;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};