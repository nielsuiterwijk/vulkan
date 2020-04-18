#include "GPUAllocator.h"

#include "graphics/GraphicsContext.h"


GPUAllocator::GPUAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = GraphicsContext::PhysicalDevice;
	allocatorInfo.device = GraphicsContext::LogicalDevice;
	allocatorInfo.instance = GraphicsContext::VulkanInstance->GetNative();
	allocatorInfo.pAllocationCallbacks = GraphicsContext::GlobalAllocator.Get();

	vmaCreateAllocator( &allocatorInfo, &_Allocator );

}

GPUAllocator::~GPUAllocator()
{
	vmaDestroyAllocator( _Allocator );
	_Allocator = nullptr;
}

int64_t GPUAllocator::BytesAllocated() const
{
	VmaStats Stats;
	vmaCalculateStats( _Allocator, &Stats );

	return Stats.total.usedBytes;
}

//The out parameter should probably be `std::unique_ptr`
bool GPUAllocator::AllocateBuffer( VkBufferCreateInfo* pCreateInfo, VmaMemoryUsage UsageFlag, SAllocatedBuffer& Out )
{
	VmaAllocationCreateInfo AllocInfo = {};
	AllocInfo.usage = UsageFlag;

	VmaAllocationInfo Info;
	if ( vmaCreateBuffer( _Allocator, pCreateInfo, &AllocInfo, &Out.Buffer, &Out.Allocation, &Info ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to allocate buffer memory!" );
		return false;
	}

	std::cout << "Allocated " << Helpers::MemorySizeToString( Info.size ) << " for buffer" << std::endl;

	return true;
	
}

bool GPUAllocator::AllocateImage( VkImageCreateInfo* pImageInfo, VmaMemoryUsage UsageFlag, SAllocatedImage& Out )
{
	VmaAllocationCreateInfo AllocInfo = {};
	AllocInfo.usage = UsageFlag;

	VmaAllocationInfo Info;
	if ( vmaCreateImage( _Allocator, pImageInfo, &AllocInfo, &Out.Image, &Out.Allocation, &Info ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to allocate image memory!" );
		return false;
	}
	std::cout << "Allocated " << Helpers::MemorySizeToString( Info.size ) << " for image" << std::endl;
	return true;
}
