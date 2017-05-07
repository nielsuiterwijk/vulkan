#include "ShaderCache.h"
#include "FragmentShader.h"
#include "graphics\GraphicsDevice.h"
#include "VertexShader.h"

VertexShader* ShaderCache::GetVertexShader(const std::string& filename)
{
	VertexShader* vertexShader = new VertexShader(filename, GraphicsContext::LogicalDevice);

	//vertexShaders.insert(std::pair<std::string, VertexShader*>(filename, vertexShader));

	return vertexShader;
}

FragmentShader* ShaderCache::GetFragmentShader(const std::string& filename)
{
	FragmentShader* fragmentShader = new FragmentShader(filename, GraphicsContext::LogicalDevice);

	//fragmentShaders.insert(std::pair<std::string, FragmentShader*>(filename, fragmentShader));

	return fragmentShader;
}
