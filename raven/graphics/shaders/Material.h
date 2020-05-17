#pragma once

#include "graphics\buffers\VulkanDescriptorPool.h"
#include "graphics\helpers\InstanceWrapper.h"

class UniformBuffer;
class VertexShader;
class FragmentShader;
class TextureSampler;

//.material file, which is a meta file to the textures and shaders needed.
class Material
{
public:
	Material( const std::string& fileName );
	~Material();

	void AddUniformBuffer( UniformBuffer* uniformBuffer );
	void UpdateUniformBuffers();

	const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages();

	const std::vector<UniformBuffer*>& GetUniformBuffers() const { return uniformBuffers; }

	std::shared_ptr<TextureSampler> GetSampler() const { return sampler; }
	std::shared_ptr<Texture2D> GetTexture() const { return texture; }

	std::shared_ptr<VertexShader> GetVertex() const { return vertex; }
	std::shared_ptr<FragmentShader> GetFragment() const { return fragment; }

	const VulkanDescriptorPool& GetDescriptorPool() const { return descriptorPool; }
	VulkanDescriptorPool& AccessDescriptorPool() { return descriptorPool; }

	bool IsLoaded() const;

	uint32_t Hash() const;

private:
	void FileLoaded( std::vector<char> fileData );

private:
	std::shared_ptr<VertexShader> vertex;
	std::shared_ptr<FragmentShader> fragment;

	VulkanDescriptorPool descriptorPool;

	std::shared_ptr<Texture2D> texture;
	std::shared_ptr<TextureSampler> sampler;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	std::vector<UniformBuffer*> uniformBuffers;
};


struct MaterialComponent
{
	std::shared_ptr<Material> _Material;

	~MaterialComponent()
	{
		_Material = nullptr;
	}

	operator std::shared_ptr<Material>() const { return _Material; }
};
