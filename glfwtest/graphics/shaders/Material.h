#pragma once

#include "graphics\helpers\InstanceWrapper.h"

#include <string>
#include <vector>

class Material
{
public:

	Material(const std::string& fileName, InstanceWrapper<VkDevice>& device);
	~Material();

private:
	void BuildShaderModule(const std::vector<char>& code, InstanceWrapper<VkShaderModule>& shaderModule, InstanceWrapper<VkDevice>& device);

private:
	InstanceWrapper<VkShaderModule> vertexShaderModule;
	InstanceWrapper<VkShaderModule> fragmentShaderModule;
};