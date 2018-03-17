#pragma once

#include "standard.h"
#include "graphics/GraphicsDevice.h"


struct CameraUBO
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class UniformBuffer
{
public:
	UniformBuffer(void* data, size_t size) : //TODO: Clarify ownership of the pointer, probably should be a weak_ptr?
		data(data),
		size(size),
		vulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, BufferType::Dynamic, data, size)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(GraphicsContext::LogicalDevice, &layoutInfo, GraphicsContext::GlobalAllocator.Get(), &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	~UniformBuffer()
	{
		vkDestroyDescriptorSetLayout(GraphicsContext::LogicalDevice, descriptorSetLayout, GraphicsContext::GlobalAllocator.Get());
		delete[] data;
		data = nullptr;
	}
	
	const VkDescriptorSetLayout& GetDescriptorSetLayout() const
	{
		return descriptorSetLayout;
	}

	void Upload()
	{
		vulkanBuffer.Map(data);
	}

	template <class T>
	T* Get() { return static_cast<T*>(data); }

private:
	void* data;
	size_t size;
	
	VkDescriptorSetLayout descriptorSetLayout;

	VulkanBuffer vulkanBuffer;
};