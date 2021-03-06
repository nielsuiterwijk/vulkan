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

	void FillBindingDescription( VkVertexInputBindingDescription& bindingDescription ) const;
	void FillAttributeDescriptions( std::vector<VkVertexInputAttributeDescription>& attributeDescriptions ) const;

	const std::vector<VkDescriptorBufferInfo>& GetBufferDescriptors() const { return _BufferDescriptors; }
	std::vector<ShaderInput>& AccessInputs() { return _Inputs; }

	void SetInputSize( uint32_t Size ) { _InputSize = Size; }
	uint32_t GetInputSize( ) const { return _InputSize; }

private:
	void ShaderLoaded( std::vector<char> fileData );
	void MetaLoaded( std::vector<char> fileData );

private:
	std::atomic_int32_t filesLeft = 0;

	std::vector<VkDescriptorBufferInfo> _BufferDescriptors;

	std::vector<ShaderInput> _Inputs;
	uint32_t _InputSize;
};
