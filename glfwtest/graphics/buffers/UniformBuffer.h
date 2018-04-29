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

//A small wrapper class that owns the concrete ubo
class UniformBuffer
{
public:
	UniformBuffer(std::shared_ptr<void> data, size_t size);
	~UniformBuffer();
	
	void Upload();

	template <class T>
	T* Get() 
	{ 
		return static_cast<T*>(data); 
	}

	const VkDescriptorBufferInfo& GetDescriptorInfo() const
	{
		return bufferInfo;
	}

private:
	std::shared_ptr<void> data;
	size_t size;
	
	VulkanBuffer vulkanBuffer;
	VkDescriptorBufferInfo bufferInfo;
};