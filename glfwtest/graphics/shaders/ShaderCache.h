#pragma once

#include <map>
#include <string>

class VertexShader;
class FragmentShader;
class Shader;

class ShaderCache
{
public:

	static VertexShader* GetVertexShader(const std::string& filename);
	static FragmentShader* GetFragmentShader(const std::string& filename);
	
private:
	//static std::map<std::string, VertexShader*> vertexShaders;
	//static std::map<std::string, FragmentShader*> fragmentShaders;
};