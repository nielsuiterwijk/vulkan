#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
public:
	VertexShader(const std::string& fileName);
	virtual ~VertexShader();

private:
	void FileLoaded(std::vector<char> fileData);

private:
};