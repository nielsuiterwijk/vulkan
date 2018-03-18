#pragma once

#include "standard.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/GraphicsContext.h"
#include "graphics/buffers/VulkanDescriptorPool.h"


struct CameraUBO
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class UniformBuffer
{
public:
	//TODO: Clarify ownership of the pointer, probably should be a weak_ptr?
	UniformBuffer(void* data, size_t size, const VulkanDescriptorPool& descriptorPool);
	~UniformBuffer();

	const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
	const VkDescriptorSet& GetDescriptorSet() const;
	void Upload();

	template <class T>
	T* Get() 
	{ 
		return static_cast<T*>(data); 
	}

private:
	void* data;
	size_t size;

	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	VulkanBuffer vulkanBuffer;
};