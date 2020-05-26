#pragma once

#include "graphics\buffers\VulkanDescriptorPool.h"
#include "graphics\helpers\InstanceWrapper.h"

class UniformBuffer;
class VertexShader;
class FragmentShader;
class TextureSampler;

//.material file, which is a meta file to the textures and shaders needed.
//TODO: This file should support changing sampler at runtime
class Material
{
public:
	Material( const std::string& fileName );
	~Material();
	
	const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const;
		
	const VertexShader* GetVertex() const { return vertex.get(); }
	const FragmentShader* GetFragment() const { return fragment.get(); }

	VertexShader* AccessVertex() { return vertex.get(); }
	FragmentShader* AccessFragment() { return fragment.get(); }

	const VulkanDescriptorPool& GetDescriptorPool() const { return descriptorPool; }
	VulkanDescriptorPool& AccessDescriptorPool() { return descriptorPool; }

	bool IsLoaded() const;

	uint32_t CalcHash() const;

	uint32_t GetPipelineHash() const { return _PipelineHash; }
	uint32_t GetSamplerHash() const { return _TextureSamplerHash; }

private:
	void FileLoaded( std::vector<char> fileData );

private:
	std::shared_ptr<VertexShader> vertex;
	std::shared_ptr<FragmentShader> fragment;

	VulkanDescriptorPool descriptorPool;

	//std::shared_ptr<TextureSampler> sampler;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	uint32_t _TextureSamplerHash = 0;
	uint32_t _PipelineHash = 0;
};


struct MaterialComponent
{
	std::shared_ptr<Material> _Material;

	std::vector<UniformBuffer*> _Buffers;

	~MaterialComponent()
	{
		_Material = nullptr;
		_Buffers.clear(); //todo delete
	}

	operator Material*() const { return _Material.get(); }
};
