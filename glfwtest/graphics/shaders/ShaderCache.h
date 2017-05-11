#pragma once

#include <map>
#include <string>
#include <memory>

class VertexShader;
class FragmentShader;
class Shader;

class ShaderCache
{
public:

	static const std::shared_ptr<VertexShader> GetVertexShader(const std::string& filename);
	static const std::shared_ptr<FragmentShader> GetFragmentShader(const std::string& filename);

	static void Destroy();

private:
	static std::map<std::string, std::shared_ptr<VertexShader> > vertexShaders;
	static std::map<std::string, std::shared_ptr<FragmentShader> > fragmentShaders;
};