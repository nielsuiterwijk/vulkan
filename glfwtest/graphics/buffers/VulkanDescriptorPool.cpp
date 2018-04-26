#include "VulkanDescriptorPool.h"

#include "graphics/GraphicsContext.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSampler.h"


VulkanDescriptorPool::VulkanDescriptorPool(uint32_t count) :
	descriptorPool(GraphicsContext::LogicalDevice, vkDestroyDescriptorPool, GraphicsContext::GlobalAllocator.Get()),
	descriptorSetLayout(GraphicsContext::LogicalDevice, vkDestroyDescriptorSetLayout, GraphicsContext::GlobalAllocator.Get()),
	currentIndex(0)
{
	CreateLayouts();
	CreatePoolAndSets(count);
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	descriptorPool = nullptr;
	descriptorSetLayout = nullptr;
	GraphicsContext::PipelineLayout = nullptr;
}

VkDescriptorPool VulkanDescriptorPool::GetNative() const
{
	return descriptorPool;
}

VkDescriptorSet VulkanDescriptorPool::GetDescriptorSet(UniformBuffer* uniformBuffer, Texture2D* texture, TextureSampler* sampler)
{
	assert(uniformBuffer != nullptr);
	assert(texture != nullptr);
	assert(sampler != nullptr);

	assert(uniformBuffer->GetDescriptorInfo().buffer != nullptr);

	VkDescriptorSet destinationDescriptorSet = descriptorSets[currentIndex];
	currentIndex = (currentIndex + 1) % descriptorSets.size();

	VkWriteDescriptorSet uboSet = {};
	uboSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboSet.dstSet = destinationDescriptorSet;
	uboSet.descriptorCount = 1;
	uboSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboSet.pBufferInfo = &uniformBuffer->GetDescriptorInfo();
	uboSet.dstBinding = 0;

	VkDescriptorImageInfo samplerDescription = {};
	samplerDescription.sampler = sampler->GetNative();
	samplerDescription.imageView = texture->GetImageView();
	samplerDescription.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkWriteDescriptorSet imageSet = {};
	imageSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	imageSet.dstSet = destinationDescriptorSet;
	imageSet.descriptorCount = 1;
	imageSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSet.pImageInfo = &samplerDescription;
	imageSet.dstBinding = 1;


	/*VkWriteDescriptorSet samplerSet = {};
	samplerSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerSet.dstSet = destinationDescriptorSet;
	samplerSet.descriptorCount = 1;
	samplerSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerSet.pImageInfo = &samplerDescription;
	samplerSet.dstBinding = 2;*/

	constexpr int setCount = 2;
	VkWriteDescriptorSet sets[setCount] = { uboSet, imageSet};
	vkUpdateDescriptorSets(GraphicsContext::LogicalDevice, setCount, sets, 0, nullptr);	

	return destinationDescriptorSet;
}

void VulkanDescriptorPool::CreateLayouts()
{

	// Binding 0 : Uniform buffer
	VkDescriptorSetLayoutBinding layoutBindingUBO = {};
	layoutBindingUBO.binding = 0;
	layoutBindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingUBO.descriptorCount = 1;
	layoutBindingUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
	layoutBindingUBO.pImmutableSamplers = nullptr;

	// Binding 1 : Image
	VkDescriptorSetLayoutBinding layoutBindingImage = {};
	layoutBindingImage.binding = 1;
	layoutBindingImage.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingImage.descriptorCount = 1;
	layoutBindingImage.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
	layoutBindingImage.pImmutableSamplers = nullptr;

	// Binding 2 : Sampler
	/*VkDescriptorSetLayoutBinding layoutBindingSampler = {};
	layoutBindingSampler.binding = 2;
	layoutBindingSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	layoutBindingSampler.descriptorCount = 1;
	layoutBindingSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
	layoutBindingSampler.pImmutableSamplers = nullptr;*/

	constexpr int bindingCount = 2;
	const VkDescriptorSetLayoutBinding bindings[bindingCount] = { layoutBindingUBO, layoutBindingImage };

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.bindingCount = bindingCount;
	descriptorLayout.pBindings = bindings;

	VkResult result = vkCreateDescriptorSetLayout(GraphicsContext::LogicalDevice, &descriptorLayout, descriptorSetLayout.AllocationCallbacks(), descriptorSetLayout.Replace());
	assert(result == VK_SUCCESS);

	//https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPipelineLayout.html
	//`a pipeline layout object which describes the complete set of resources that CAN be accessed by a pipeline.`
	//Thats why you can bind any potential descriptors, even if you don't use them
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

		result = vkCreatePipelineLayout(GraphicsContext::LogicalDevice, &pipelineLayoutCreateInfo, GraphicsContext::PipelineLayout.AllocationCallbacks(), GraphicsContext::PipelineLayout.Replace());
		assert(result == VK_SUCCESS);
	}

}

void VulkanDescriptorPool::CreatePoolAndSets(uint32_t count)
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = count;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = count;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = count;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkResult result = vkCreateDescriptorPool(GraphicsContext::LogicalDevice, &descriptorPoolInfo, descriptorPool.AllocationCallbacks(), descriptorPool.Replace());
	assert(result == VK_SUCCESS);

	descriptorSets.resize(count);

	for (std::size_t i = 0; i < descriptorSets.size(); ++i)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		result = vkAllocateDescriptorSets(GraphicsContext::LogicalDevice, &allocInfo, &descriptorSets[i]);
		assert(result == VK_SUCCESS);
	}
}