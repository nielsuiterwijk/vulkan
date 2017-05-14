#include "ShaderCache.h"
#include "FragmentShader.h"
#include "graphics\GraphicsDevice.h"
#include "VertexShader.h"

std::map<std::string, std::shared_ptr<VertexShader> > ShaderCache::vertexShaders;
std::map<std::string, std::shared_ptr<FragmentShader> > ShaderCache::fragmentShaders;

const std::shared_ptr<VertexShader> ShaderCache::GetVertexShader(const std::string& filename)
{
	std::map<std::string, std::shared_ptr<VertexShader>>::iterator it = vertexShaders.find(filename);

	if (it != vertexShaders.end())
	{
		return it->second;
	}
	else
	{
		std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(filename);

		vertexShaders.insert(std::make_pair(filename, vertexShader));

		return vertexShader;
	}
}

const std::shared_ptr<FragmentShader> ShaderCache::GetFragmentShader(const std::string& filename)
{
	std::map<std::string, std::shared_ptr<FragmentShader>>::iterator it = fragmentShaders.find(filename);

	if (it != fragmentShaders.end())
	{
		return it->second;
	}
	else
	{
		std::shared_ptr<FragmentShader> fragmentShader = std::make_shared<FragmentShader>(filename);

		fragmentShaders.insert(std::make_pair(filename, fragmentShader));

		return fragmentShader;
	}
}

void ShaderCache::Destroy()
{
	std::map<std::string, std::shared_ptr<FragmentShader>>::iterator fragments;
	for (fragments = fragmentShaders.begin(); fragments != fragmentShaders.end(); fragments++)
	{
		fragments->second = nullptr;
	}
	fragmentShaders.clear();

	std::map<std::string, std::shared_ptr<VertexShader>>::iterator vertices;
	for (vertices = vertexShaders.begin(); vertices != vertexShaders.end(); vertices++)
	{
		vertices->second = nullptr;
	}
	vertexShaders.clear();
}
