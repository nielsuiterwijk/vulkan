#pragma once

#include "graphics\helpers\InstanceWrapper.h"

#include <string>
#include <vector>

class VertexShader;
class FragmentShader;

//.material file, which is a meta file to the textures and shaders needed.
class Material
{
public:
	Material(const std::string& fileName);
	~Material();

private:
	VertexShader* vertex;
	FragmentShader* fragment;
};