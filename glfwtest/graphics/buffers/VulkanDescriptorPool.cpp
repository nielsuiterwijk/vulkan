#include "VulkanDescriptorPool.h"

#include "graphics/GraphicsContext.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/shaders/FragmentShader.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSampler.h"

static constexpr int32_t VULKAN_NUM_DESCRIPTOR_SETS = 4;
static constexpr int32_t VULKAN_NUM_BINDINGS = 16;
static constexpr int32_t VULKAN_NUM_SETS_PER_POOL = 16;

VulkanDescriptorPool::VulkanDescriptorPool() :
	descriptorPool( GraphicsContext::LogicalDevice, vkDestroyDescriptorPool, GraphicsContext::GlobalAllocator.Get() ),
	pipelineLayout( GraphicsContext::LogicalDevice, vkDestroyPipelineLayout, GraphicsContext::GlobalAllocator.Get() ),
	currentIndex( 0 )
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	descriptorPool = nullptr;
	pipelineLayout = nullptr;
}

VkDescriptorPool VulkanDescriptorPool::GetNative() const
{
	return descriptorPool;
}

// TODO: This needs to take in material and generate descriptor set based on the generated .vert.json file
VkDescriptorSet VulkanDescriptorPool::RetrieveDescriptorSet( std::shared_ptr<Material> material, Texture2D* texture, TextureSampler* sampler )
{
	assert( material != nullptr );
	assert( texture != nullptr );
	assert( sampler != nullptr );

	std::shared_ptr<VertexShader> pVertexShader = material->GetVertex();
	std::shared_ptr<FragmentShader> pFragmentShader = material->GetFragment();

	VkDescriptorSet destinationDescriptorSet = descriptorSets[ currentIndex ];
	currentIndex = ( currentIndex + 1 ) % descriptorSets.size();

	std::vector<VkWriteDescriptorSet> sets;
	sets.reserve( pVertexShader->GetResourceLayout().size() + pFragmentShader->GetResourceLayout().size() );

	assert( pVertexShader->GetResourceLayout().size() == material->GetUniformBuffers().size() );
	for ( int32_t i = 0; i < pVertexShader->GetResourceLayout().size(); i++ )
	{
		const ResourceLayout& resourceLayout = pVertexShader->GetResourceLayout()[ i ];

		const UniformBuffer* pUbo = material->GetUniformBuffers()[ i ];
		assert( pUbo->GetDescriptorInfo().range == pVertexShader->GetBufferDescriptors()[ i ].range );

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = destinationDescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &pUbo->GetDescriptorInfo();
		writeDescriptorSet.dstBinding = resourceLayout.BindingSlot;

		sets.push_back( writeDescriptorSet );
	}

	assert( pFragmentShader->GetResourceLayout().size() == 1 );
	for ( int32_t i = 0; i < pFragmentShader->GetResourceLayout().size(); i++ )
	{
		const ResourceLayout& resourceLayout = pFragmentShader->GetResourceLayout()[ i ];

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
		imageSet.dstBinding = resourceLayout.BindingSlot;

		sets.push_back( imageSet );

		break;
	}

	vkUpdateDescriptorSets( GraphicsContext::LogicalDevice, static_cast<uint32_t>( sets.size() ), sets.data(), 0, nullptr );

	return destinationDescriptorSet;
}

void VulkanDescriptorPool::SetupBindings( std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<FragmentShader> pFragmentShader )
{
	std::vector<ResourceLayout> resourceLayouts;
	resourceLayouts.reserve( pVertexShader->GetResourceLayout().size() + pFragmentShader->GetResourceLayout().size() );
	resourceLayouts.insert( resourceLayouts.end(), pVertexShader->GetResourceLayout().begin(), pVertexShader->GetResourceLayout().end() );
	resourceLayouts.insert( resourceLayouts.end(), pFragmentShader->GetResourceLayout().begin(), pFragmentShader->GetResourceLayout().end() );

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for ( unsigned i = 0; i < VULKAN_NUM_BINDINGS; i++ )
	{
		if ( i >= resourceLayouts.size() )
			continue;

		ResourceLayout resourceLayout = resourceLayouts[ i ];

		uint32_t ArraySize = resourceLayout.ArraySize;

		VkSampler sampler = VK_NULL_HANDLE;

		uint32_t Size = static_cast<uint32_t>( ArraySize * VULKAN_NUM_SETS_PER_POOL );

		bindings.push_back( { resourceLayout.BindingSlot,
							  resourceLayout.Type,
							  ArraySize,
							  resourceLayout.ShaderStage,
							  sampler != VK_NULL_HANDLE ? &sampler : nullptr } );

		poolSizes.push_back( { resourceLayout.Type, Size } );
	}

	VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	if ( !bindings.empty() )
	{
		info.bindingCount = static_cast<uint32_t>( bindings.size() );
		info.pBindings = bindings.data();
	}

	VkResult result = vkCreateDescriptorSetLayout( GraphicsContext::LogicalDevice,
												   &info,
												   descriptorSetLayout.AllocationCallbacks(),
												   descriptorSetLayout.Replace() );
	assert( result == VK_SUCCESS );

	// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPipelineLayout.html
	//`a pipeline layout object which describes the complete set of resources that CAN be accessed by a pipeline.`
	// Thats why you can bind any potential descriptors, even if you don't use them
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

		result = vkCreatePipelineLayout( GraphicsContext::LogicalDevice,
										 &pipelineLayoutCreateInfo,
										 pipelineLayout.AllocationCallbacks(),
										 pipelineLayout.Replace() );
		assert( result == VK_SUCCESS );
	}

	CreatePoolAndSets();
}

void VulkanDescriptorPool::CreatePoolAndSets()
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = VULKAN_NUM_SETS_PER_POOL;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkResult result = vkCreateDescriptorPool( GraphicsContext::LogicalDevice,
											  &descriptorPoolInfo,
											  descriptorPool.AllocationCallbacks(),
											  descriptorPool.Replace() );
	assert( result == VK_SUCCESS );

	descriptorSets.resize( VULKAN_NUM_SETS_PER_POOL );

	VkDescriptorSetLayout layouts[ VULKAN_NUM_SETS_PER_POOL ];
	std::fill( std::begin( layouts ), std::end( layouts ), descriptorSetLayout );

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = VULKAN_NUM_SETS_PER_POOL;
	allocInfo.pSetLayouts = layouts;

	result = vkAllocateDescriptorSets( GraphicsContext::LogicalDevice,
									   &allocInfo,
									   descriptorSets.data() );
	assert( result == VK_SUCCESS );
}
