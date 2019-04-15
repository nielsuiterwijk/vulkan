#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
public:
	FragmentShader( const std::string& fileName );
	virtual ~FragmentShader();

	virtual bool IsLoaded() const override { return filesLeft.load( std::memory_order_relaxed ) == 0; }

private:
	void ShaderLoaded( std::vector<char> fileData );
	void MetaLoaded( std::vector<char> fileData );

private:
	std::atomic_int32_t filesLeft = 0;
};
