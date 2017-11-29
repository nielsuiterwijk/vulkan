#include "GPUAllocator.h"

#include "graphics/GraphicsDevice.h"

GPUAllocator::GPUAllocator(int32_t size, int32_t alignment)
{
	memoryProperties = new VkPhysicalDeviceMemoryProperties();
	vkGetPhysicalDeviceMemoryProperties(GraphicsContext::PhysicalDevice, memoryProperties);
}

GPUAllocator::~GPUAllocator()
{

}

void GPUAllocator::Allocate(VulkanBuffer& buffer)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(GraphicsContext::LogicalDevice, buffer.GetNative(), &memoryRequirements);

	int32_t memoryType = FindProperties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	assert(memoryType >= 0);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = memoryType;

	if (vkAllocateMemory(GraphicsContext::LogicalDevice, &allocInfo, GraphicsContext::GlobalAllocator.Get(), buffer.GetDeviceMemory()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}


}

// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`
int32_t GPUAllocator::FindProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties)
{
	const uint32_t memoryCount = memoryProperties->memoryTypeCount;

	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

		const VkMemoryPropertyFlags properties = memoryProperties->memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;

		if (isRequiredMemoryType && hasRequiredProperties)
			return static_cast<int32_t>(memoryIndex);
	}

	// failed to find memory type
	return -1;
}