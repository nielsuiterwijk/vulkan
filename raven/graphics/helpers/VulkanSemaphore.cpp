#include "VulkanSemaphore.h"

#include "graphics\GraphicsContext.h"

VulkanSemaphore::VulkanSemaphore() :
	semaphore( GraphicsContext::LogicalDevice, vkDestroySemaphore, GraphicsContext::LocalAllocator )
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if ( vkCreateSemaphore( GraphicsContext::LogicalDevice, &semaphoreInfo, semaphore.AllocationCallbacks(), semaphore.Replace() ) != VK_SUCCESS )
	{
		ASSERT_FAIL( "failed to create semaphores!" );
	}
}

VulkanSemaphore::~VulkanSemaphore()
{
	semaphore = nullptr;
}

const InstanceWrapper<VkSemaphore>& VulkanSemaphore::GetNative() const
{
	return semaphore;
}
