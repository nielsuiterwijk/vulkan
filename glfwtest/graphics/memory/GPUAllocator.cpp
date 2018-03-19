#include "GPUAllocator.h"

#include "graphics/GraphicsContext.h"

GPUAllocator::GPUAllocator(int32_t size, int32_t alignment)
{
	memoryProperties = new VkPhysicalDeviceMemoryProperties();
	vkGetPhysicalDeviceMemoryProperties(GraphicsContext::PhysicalDevice, memoryProperties);

	const VkDeviceSize heapSize1 = memoryProperties->memoryHeaps[0].size;
	const VkDeviceSize heapSize2 = memoryProperties->memoryHeaps[1].size;

	std::cout << "RAM: " << Helpers::MemorySizeToString(heapSize2) << std::endl;
	std::cout << "VRAM: " << Helpers::MemorySizeToString(heapSize1) << std::endl;

	/*VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = 1024*1024*1024;
	allocInfo.memoryTypeIndex = 7;

	VkDeviceMemory memory;

	if (vkAllocateMemory(GraphicsContext::LogicalDevice, &allocInfo, GraphicsContext::GlobalAllocator.Get(), &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}*/
}

GPUAllocator::~GPUAllocator()
{

}

void GPUAllocator::Allocate(const VkBuffer& buffer, VkDeviceMemory* memory, VkMemoryPropertyFlags requiredProperties)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(GraphicsContext::LogicalDevice, buffer, &memoryRequirements);

	int32_t memoryType = FindProperties(memoryRequirements.memoryTypeBits, requiredProperties);

	assert(memoryType >= 0);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = memoryType;

	if (vkAllocateMemory(GraphicsContext::LogicalDevice, &allocInfo, GraphicsContext::GlobalAllocator.Get(), memory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	std::cout << "Allocated " << Helpers::MemorySizeToString(memoryRequirements.size) << " bytes." << std::endl;
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