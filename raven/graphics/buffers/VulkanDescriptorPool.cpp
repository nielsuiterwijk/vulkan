#include "VulkanDescriptorPool.h"

#include "graphics/GraphicsContext.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/shaders/FragmentShader.h"
#include "graphics/shaders/Material.h"
#include "graphics/shaders/VertexShader.h"
#include "graphics/textures/Texture2D.h"
#include "graphics/textures/TextureSamplerCache.h"

static constexpr int32_t VULKAN_NUM_SETS_PER_POOL = 90;

VulkanDescriptorPool::VulkanDescriptorPool()
	: descriptorPool( GraphicsContext::LogicalDevice, vkDestroyDescriptorPool, GraphicsContext::LocalAllocator )
	, _DescriptorSetLayout( GraphicsContext::LogicalDevice, vkDestroyDescriptorSetLayout, GraphicsContext::LocalAllocator )
	, pipelineLayout( GraphicsContext::LogicalDevice, vkDestroyPipelineLayout, GraphicsContext::LocalAllocator )
	, currentIndex( 0 )
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	descriptorPool = nullptr;
	pipelineLayout = nullptr;
	_DescriptorSetLayout = nullptr;
}

VkDescriptorSet VulkanDescriptorPool::RetrieveDescriptorSet( const Material* pMaterial, const std::vector<std::shared_ptr<Texture2D>>& textures, const std::vector<UniformBuffer*>& buffers )
{
	std::vector<Texture2D*> texturePtrs;

	for ( const std::shared_ptr<Texture2D>& Texture : textures )
	{
		texturePtrs.push_back( Texture.get() );
	}

	return RetrieveDescriptorSet( pMaterial, texturePtrs, buffers );
}

VkDescriptorSet VulkanDescriptorPool::RetrieveDescriptorSet( const Material* pMaterial, const std::vector<Texture2D*>& textures, const std::vector<UniformBuffer*>& buffers )
{
	ASSERT( pMaterial != nullptr );

	const VertexShader* pVertexShader = pMaterial->GetVertex();
	const FragmentShader* pFragmentShader = pMaterial->GetFragment();

	//todo: use atomic?
	VkDescriptorSet destinationDescriptorSet = _DescriptorSets[ currentIndex ];
	currentIndex = ( currentIndex + 1 ) % _DescriptorSets.size();

	std::vector<VkWriteDescriptorSet> sets;
	sets.reserve( pVertexShader->GetResourceLayout().size() + pFragmentShader->GetResourceLayout().size() );

	ASSERT( pVertexShader->GetResourceLayout().size() == buffers.size() );
	for ( int32_t i = 0; i < pVertexShader->GetResourceLayout().size(); i++ )
	{
		const ResourceLayout& resourceLayout = pVertexShader->GetResourceLayout()[ i ];
		const UniformBuffer* pUbo = buffers[ i ];

		ASSERT( pUbo->GetDescriptorInfo().range == pVertexShader->GetBufferDescriptors()[ i ].range );

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = destinationDescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &pUbo->GetDescriptorInfo();
		writeDescriptorSet.dstBinding = resourceLayout.BindingSlot;

		sets.push_back( writeDescriptorSet );
	}

	ASSERT( pFragmentShader->GetResourceLayout().size() == textures.size() );
	for ( int32_t i = 0; i < pFragmentShader->GetResourceLayout().size(); i++ )
	{
		const ResourceLayout& resourceLayout = pFragmentShader->GetResourceLayout()[ i ];

		{
			//TODO: How does this even work? Object goes out of scope and should be cleared from stack.
			VkDescriptorImageInfo samplerDescription = {};
			samplerDescription.sampler = TextureSamplerCache::GetSampler( pMaterial->GetSamplerHash() );
			samplerDescription.imageView = textures[ i ]->GetImageView();
			samplerDescription.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ASSERT( samplerDescription.sampler );
			ASSERT( samplerDescription.imageView );

			VkWriteDescriptorSet imageSet = {};
			imageSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			imageSet.dstSet = destinationDescriptorSet;
			imageSet.descriptorCount = 1;
			imageSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageSet.pImageInfo = &samplerDescription;
			imageSet.dstBinding = resourceLayout.BindingSlot;

			sets.push_back( imageSet );
		}
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
	bindings.reserve( resourceLayouts.size() );
	_PoolSizes.reserve( resourceLayouts.size() );
	for ( const ResourceLayout& resourceLayout : resourceLayouts )
	{
		uint32_t ArraySize = resourceLayout.ArraySize;

		VkSampler sampler = VK_NULL_HANDLE;

		uint32_t Size = static_cast<uint32_t>( ArraySize * VULKAN_NUM_SETS_PER_POOL );

		bindings.push_back( { resourceLayout.BindingSlot, resourceLayout.Type, ArraySize, resourceLayout.ShaderStage, sampler != VK_NULL_HANDLE ? &sampler : nullptr } );

		_PoolSizes.push_back( { resourceLayout.Type, Size } );
	}

	VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	if ( !bindings.empty() )
	{
		info.bindingCount = static_cast<uint32_t>( bindings.size() );
		info.pBindings = bindings.data();
	}

	VkResult result = vkCreateDescriptorSetLayout( GraphicsContext::LogicalDevice, &info, _DescriptorSetLayout.AllocationCallbacks(), _DescriptorSetLayout.Replace() );
	ASSERT( result == VK_SUCCESS );

	std::cout << "Pool layout: " << (VkDescriptorSetLayout)_DescriptorSetLayout << std::endl;

	// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPipelineLayout.html
	//`a pipeline layout object which describes the complete set of resources that CAN be accessed by a pipeline.`
	// Thats why you can bind any potential descriptors, even if you don't use them
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &_DescriptorSetLayout;

		result = vkCreatePipelineLayout( GraphicsContext::LogicalDevice, &pipelineLayoutCreateInfo, pipelineLayout.AllocationCallbacks(), pipelineLayout.Replace() );
		ASSERT( result == VK_SUCCESS );
	}

	std::cout << "Pool pipeline: " << (VkPipelineLayout)pipelineLayout << std::endl;

	CreatePoolAndSets();
}

void VulkanDescriptorPool::CreatePoolAndSets()
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>( _PoolSizes.size() );
	descriptorPoolInfo.pPoolSizes = _PoolSizes.data();
	descriptorPoolInfo.maxSets = VULKAN_NUM_SETS_PER_POOL;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkResult result = vkCreateDescriptorPool( GraphicsContext::LogicalDevice, &descriptorPoolInfo, descriptorPool.AllocationCallbacks(), descriptorPool.Replace() );
	ASSERT( result == VK_SUCCESS );

	_DescriptorSets.resize( VULKAN_NUM_SETS_PER_POOL );

	VkDescriptorSetLayout layouts[ VULKAN_NUM_SETS_PER_POOL ];
	std::fill( std::begin( layouts ), std::end( layouts ), _DescriptorSetLayout );

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = VULKAN_NUM_SETS_PER_POOL;
	allocInfo.pSetLayouts = layouts;

	result = vkAllocateDescriptorSets( GraphicsContext::LogicalDevice, &allocInfo, _DescriptorSets.data() );
	ASSERT( result == VK_SUCCESS );
}
