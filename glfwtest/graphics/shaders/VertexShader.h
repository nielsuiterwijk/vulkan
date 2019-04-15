#pragma once

#include "Shader.h"
#include "graphics/models/Mesh.h"
#include <atomic>

struct ShaderInput
{
#if DEBUG
	std::string type;
	std::string name;
#endif
	int location;

	VkFormat typeFormat;
	uint32_t offset;
};

class VertexShader : public Shader
{
public:
	VertexShader( const std::string& fileName );
	virtual ~VertexShader();

	virtual bool IsLoaded() const override { return filesLeft.load( std::memory_order_relaxed ) == 0; }

	void GetBindingDescription( VkVertexInputBindingDescription& bindingDescription );
	void GetAttributeDescriptions( std::vector<VkVertexInputAttributeDescription>& attributeDescriptions );

	const std::vector<VkDescriptorBufferInfo>& GetBufferDescriptors() const { return bufferDescriptors; }

private:
	void ShaderLoaded( std::vector<char> fileData );
	void MetaLoaded( std::vector<char> fileData );

private:
	std::atomic_int32_t filesLeft = 0;

	std::vector<VkDescriptorBufferInfo> bufferDescriptors;

	std::vector<ShaderInput> inputs;

#if DEBUG
	std::string shaderFileName;
#endif
};
