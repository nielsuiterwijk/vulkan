#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(void* data, size_t size, const VulkanDescriptorPool& descriptorPool) : 
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

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool.Get();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(GraphicsContext::LogicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = vulkanBuffer.GetNative();
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0; //defined in the shader, for example: layout(binding = 0)
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

	vkUpdateDescriptorSets(GraphicsContext::LogicalDevice, 1, &descriptorWrite, 0, nullptr);
}

UniformBuffer::~UniformBuffer()
{
	vkDestroyDescriptorSetLayout(GraphicsContext::LogicalDevice, descriptorSetLayout, GraphicsContext::GlobalAllocator.Get());
	delete[] data;
	data = nullptr;
}

const VkDescriptorSetLayout& UniformBuffer::GetDescriptorSetLayout() const
{
	return descriptorSetLayout;
}

const VkDescriptorSet& UniformBuffer::GetDescriptorSet() const
{
	return descriptorSet;
}

void UniformBuffer::Upload()
{
	vulkanBuffer.Map(data);
}