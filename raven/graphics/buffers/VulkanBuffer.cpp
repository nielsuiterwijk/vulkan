#include "VulkanBuffer.h"

#include "graphics/GraphicsContext.h"
#include "graphics/memory/GPUAllocator.h"

#include <vulkan/vulkan.h>

VulkanBuffer::VulkanBuffer( VkBufferUsageFlags Flags, BufferType BufferType, void* BufferData, VkDeviceSize Size ) :
	_Size( Size ),
	_BufferType( BufferType )
{

	switch ( BufferType )
	{
	case BufferType::Static:
	{
		VulkanBuffer Tmp( Flags, BufferType::Staging, BufferData, Size );
		
		SetupLocalStaticBuffer( Flags );

		Tmp.CopyToBufferAndClear(*this);
		break;
	}
	case BufferType::Dynamic:
		SetupLocalDynamicBuffer( Flags );
		Map( BufferData );
		break;

	case BufferType::Staging:
		SetupStagingBuffer();
		Map( BufferData );
		break;
	default:
		assert( false && "Not implemented." );
		break;
	}
}

VulkanBuffer::~VulkanBuffer()
{
	{
		std::scoped_lock<std::mutex> Lock( GraphicsContext::QueueLock ); //todo: replace with own variant for RWLOCK
		vkQueueWaitIdle( GraphicsContext::GraphicsQueue );
	}
	Free();
}

void VulkanBuffer::Free()
{
	if ( _NativeBuffer.Buffer != nullptr )
	{
		std::cout << "freeing device buffer of " << Helpers::MemorySizeToString( _Size ) << " bytes" << std::endl;

		vmaDestroyBuffer( GraphicsContext::DeviceAllocator->Get(), _NativeBuffer.Buffer, _NativeBuffer.Allocation ); //TODO: Maybe also have this go through the GPUAllocator class?

		_NativeBuffer.Buffer = nullptr;
		_NativeBuffer.Allocation = nullptr;
	}
}

void VulkanBuffer::SetupLocalDynamicBuffer( VkBufferUsageFlags flags )
{
	VkBufferCreateInfo localBufferInfo = {};
	localBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	localBufferInfo.size = _Size;
	localBufferInfo.usage = flags;
	localBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	GraphicsContext::DeviceAllocator->AllocateBuffer( &localBufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, _NativeBuffer );
}

void VulkanBuffer::SetupLocalStaticBuffer( VkBufferUsageFlags flags )
{
	VkBufferCreateInfo localBufferInfo = {};
	localBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	localBufferInfo.size = _Size;
	localBufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	localBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	GraphicsContext::DeviceAllocator->AllocateBuffer( &localBufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, _NativeBuffer );
}

void VulkanBuffer::SetupStagingBuffer()
{
	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = _Size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	GraphicsContext::DeviceAllocator->AllocateBuffer( &stagingBufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, _NativeBuffer );
}

void VulkanBuffer::CopyToBufferAndClear( VulkanBuffer& Destination )
{
	assert( Destination.GetType() != BufferType::Staging );

	auto commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	commandBuffer->StartRecording( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = _Size;
	vkCmdCopyBuffer( commandBuffer->GetNative(), GetNative(), Destination.GetNative(), 1, &copyRegion );

	commandBuffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->GetNative();

	{
		std::scoped_lock<std::mutex> Lock( GraphicsContext::QueueLock );
		//TODO: This causes waits, needs to be async
		vkQueueSubmit( GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( GraphicsContext::TransportQueue );
	}

	GraphicsContext::CommandBufferPoolTransient->Free( commandBuffer );

	Free();
}

void VulkanBuffer::CopyToImageAndClear( VkImage image, uint32_t width, uint32_t height )
{
	auto commandBuffer = GraphicsContext::CommandBufferPoolTransient->Create();

	commandBuffer->StartRecording( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage( commandBuffer->GetNative(), GetNative(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

	commandBuffer->StopRecording();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->GetNative();

	GraphicsContext::QueueLock.lock();
	//TODO: This causes waits, needs to be async
	vkQueueSubmit( GraphicsContext::TransportQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( GraphicsContext::TransportQueue );

	GraphicsContext::QueueLock.unlock();

	GraphicsContext::CommandBufferPoolTransient->Free( commandBuffer );

	Free();
}

void VulkanBuffer::Map( void* bufferData, VkDeviceSize sizeToMap ) const //TODO: this name doesnt tell the truth, it aint a persistent mapping. 'Update' maybe?
{
	if ( bufferData == nullptr )
		return;

	if ( sizeToMap == -1 )
		sizeToMap = _Size;

	assert( sizeToMap <= _Size );

	VkResult result = VK_SUCCESS;
	void* vulkanVirtualMappedMemoryAddress;

	VmaAllocator DeviceAllocator = GraphicsContext::DeviceAllocator->Get();

	switch ( _BufferType )
	{
	case BufferType::Static:
	case BufferType::Staging:
	case BufferType::Dynamic:
		/*result = vkMapMemory( GraphicsContext::LogicalDevice, _StagingMemory, 0, sizeToMap, 0, &vulkanVirtualMappedMemoryAddress );
		assert( result == VK_SUCCESS );
		memcpy( vulkanVirtualMappedMemoryAddress, bufferData, sizeToMap );
		vkUnmapMemory( GraphicsContext::LogicalDevice, _StagingMemory );*/
		result = vmaMapMemory( DeviceAllocator, _NativeBuffer.Allocation, &vulkanVirtualMappedMemoryAddress );
		assert( result == VK_SUCCESS );
		memcpy( vulkanVirtualMappedMemoryAddress, bufferData, sizeToMap );
		vmaUnmapMemory( DeviceAllocator, _NativeBuffer.Allocation );

		break;
	default:
		assert( false && "Not implemented." );
		break;
	}
}
