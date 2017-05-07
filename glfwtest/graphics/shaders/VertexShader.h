#pragma once

#include "Shader.h"

class VertexShader : Shader
{
public:
	VertexShader(const std::string& fileName, const VkDevice& device);
	virtual ~VertexShader();

private:
};