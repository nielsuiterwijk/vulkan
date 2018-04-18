#pragma once

#include "graphics\helpers\InstanceWrapper.h"
#include "graphics\buffers\VulkanDescriptorPool.h"

#include <string>
#include <vector>
#include <memory>

class UniformBuffer;
class VertexShader;
class FragmentShader;

//.material file, which is a meta file to the textures and shaders needed.
class Material
{
public:
	Material(const std::string& fileName);
	~Material();

	const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages();

	const std::vector<UniformBuffer*>& GetUniformBuffers() const;

	bool IsLoaded() const;

private:
	std::shared_ptr<VertexShader> vertex;
	std::shared_ptr<FragmentShader> fragment;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VulkanDescriptorPool descripterPool;

	std::vector<UniformBuffer*> uniformBuffers;
};