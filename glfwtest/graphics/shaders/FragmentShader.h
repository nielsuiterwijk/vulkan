#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
public:
	FragmentShader(const std::string& fileName);
	virtual ~FragmentShader();


	virtual bool IsLoaded() const override { return isLoaded; }

private:
	void FileLoaded(std::vector<char> fileData);

private:
	bool isLoaded = false;
};