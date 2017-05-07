#pragma once

#include "Shader.h"

class FragmentShader : Shader
{
public:
	FragmentShader(const std::string& fileName, const VkDevice& device);
	virtual ~FragmentShader();

private:
};