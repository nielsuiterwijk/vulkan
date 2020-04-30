#pragma once

#include "Shader.h"

#include <atomic>

class FragmentShader : public Shader
{
public:
	FragmentShader( const std::string& fileName );
	virtual ~FragmentShader();

	virtual bool IsLoaded() const override { return _FilesLeft.load( std::memory_order_relaxed ) == 0; }

private:
	void ShaderLoaded( std::vector<char> fileData );
	void MetaLoaded( std::vector<char> fileData );

private:
	std::atomic<int32_t> _FilesLeft = 0;
};
