#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
public:
	FragmentShader(const std::string& fileName);
	virtual ~FragmentShader();

private:
	void FileLoaded(std::vector<char> fileData);

private:
};