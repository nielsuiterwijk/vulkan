#pragma once

#include "graphics/helpers/VulkanHelpers.h"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <vulkan/vulkan.h>

#include "helpers/Singleton.h"
#include "helpers/Helpers.h"
#include "strings/string_utils.h"

class VulkanAllocator
{
private:
	struct AllocationTracker
	{
	public:
		AllocationTracker( VkSystemAllocationScope scope ) :
			scope( scope ),
			totalMemoryAllocated( 0 ),
			previousTotalMemoryAllocated( 0 ),
			totalAllocations( 0 ),
			previousTotalAllocations( 0 ),
			totalReallocations( 0 ),
			previousTotalReallocations( 0 )
		{
		}

		std::map<void*, size_t> allocatedMemory;

		int64_t totalMemoryAllocated;
		int64_t previousTotalMemoryAllocated;

		int64_t totalAllocations;
		int64_t previousTotalAllocations;

		int64_t totalReallocations;
		int64_t previousTotalReallocations;

		VkSystemAllocationScope scope;
	};

public:
	VulkanAllocator() :
		vulkanAllocator()
	{
		vulkanAllocator.pUserData = (void*)this;
		vulkanAllocator.pfnAllocation = &VulkanAllocator::Allocate;
		vulkanAllocator.pfnReallocation = &VulkanAllocator::Reallocate;
		vulkanAllocator.pfnFree = &VulkanAllocator::Free;
		vulkanAllocator.pfnInternalAllocation = &VulkanAllocator::InternalAllocationNotification;
		vulkanAllocator.pfnInternalFree = &VulkanAllocator::InternalFreeNotification;
		
		trackers.push_back( AllocationTracker( VK_SYSTEM_ALLOCATION_SCOPE_COMMAND ) );
		trackers.push_back( AllocationTracker( VK_SYSTEM_ALLOCATION_SCOPE_OBJECT ) );
		trackers.push_back( AllocationTracker( VK_SYSTEM_ALLOCATION_SCOPE_CACHE ) );
		trackers.push_back( AllocationTracker( VK_SYSTEM_ALLOCATION_SCOPE_DEVICE ) );
		trackers.push_back( AllocationTracker( VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE ) );

		std::cout << "Vulkan CPU Allocator" << std::endl;
	}

	~VulkanAllocator()
	{
		vulkanAllocator.pUserData = nullptr;
		vulkanAllocator.pfnAllocation = nullptr;
		vulkanAllocator.pfnReallocation = nullptr;
		vulkanAllocator.pfnFree = nullptr;
		vulkanAllocator.pfnInternalAllocation = nullptr;
		vulkanAllocator.pfnInternalFree = nullptr;

		trackers.clear();

		std::cout << "~Vulkan CPU Allocator" << std::endl;
	}

	int64_t BytesAllocated() const
	{
		int64_t VulkanMemoryUsage = 0;


		for ( size_t i = 0; i < trackers.size(); i++ )
		{
			int64_t memoryAllocated = trackers[ i ].totalMemoryAllocated;

			VulkanMemoryUsage += memoryAllocated;
		}

		return VulkanMemoryUsage;
	}

	void PrintStats()
	{
		int64_t vulkanMemoryUsage = 0;


		for ( size_t i = 0; i < trackers.size(); i++ )
		{
			int64_t MemoryAllocated = trackers[ i ].totalMemoryAllocated;
			int64_t previousMemoryAllocated = trackers[ i ].previousTotalMemoryAllocated;
			int64_t previousTotalAllocations = trackers[ i ].previousTotalAllocations;
			int64_t previousTotalReallocations = trackers[ i ].previousTotalReallocations;

			std::string DeltaAllocatedString;
			std::string AllocCount;
			std::string ReallocCount;
				
			{
				int32_t Delta = static_cast<int32_t>( MemoryAllocated - previousMemoryAllocated );
				StringUtils::StringFormat<256> Tmp( "%s%d", ( Delta ) > 0 ? "+" : "", Delta );
				DeltaAllocatedString = Tmp;
			}
			
			{
				int32_t Delta = static_cast<int32_t>( trackers[ i ].totalAllocations - previousTotalAllocations );
				StringUtils::StringFormat<256> DeltaTotal( "%s%d", ( Delta ) > 0 ? "+" : "", Delta );
				StringUtils::StringFormat<256> Tmp( "allocs: %d (%s)", trackers[ i ].totalAllocations, DeltaTotal.c_str() );
				AllocCount = Tmp;
			}

			{
				int32_t Delta = static_cast<int32_t>( trackers[ i ].totalReallocations - previousTotalReallocations );
				StringUtils::StringFormat<256> DeltaTotal( "%s%d", ( Delta ) > 0 ? "+" : "", Delta );
				StringUtils::StringFormat<256> Tmp( "reallocs: %d (%s)", trackers[ i ].totalReallocations, DeltaTotal.c_str() );
				ReallocCount = Tmp;
			}

			std::stringstream ss;
			
			std::string allocName = std::string( Vulkan::GetAllocationScopeName( trackers[ i ].scope ) ) + " allocated: ";
			ss << std::setw( 23 ) << allocName;
			allocName = ss.str();
			
			std::cout << "[Vulkan] " << allocName << std::left << std::setw( 28 ) << 
				Helpers::MemorySizeToString( MemoryAllocated ) + ( " (" + DeltaAllocatedString + " bytes.)" ) << std::left << std::setw( 18 ) << 
				AllocCount << std::left << std::setw( 18 ) << 
				ReallocCount << std::endl;

			trackers[ i ].previousTotalMemoryAllocated = trackers[ i ].totalMemoryAllocated;
			trackers[ i ].previousTotalAllocations = trackers[ i ].totalAllocations;
			trackers[ i ].previousTotalReallocations = trackers[ i ].totalReallocations;

			vulkanMemoryUsage += MemoryAllocated;
		}

		std::cout << "[Vulkan] total memory usage: " << Helpers::MemorySizeToString( vulkanMemoryUsage ) << "." << std::endl;
		std::cout << "" << std::endl;
	}

	operator VkAllocationCallbacks*() { return &vulkanAllocator; }

private:
	static void* VKAPI_CALL Allocate( void* userData, size_t size, size_t alignment, VkSystemAllocationScope scope )
	{
		return static_cast<VulkanAllocator*>( userData )->Allocate( size, alignment, scope );
	}

	static void* VKAPI_CALL Reallocate( void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope )
	{
		return static_cast<VulkanAllocator*>( userData )->Reallocate( original, size, alignment, scope );
	}

	static void VKAPI_CALL Free( void* userData, void* data )
	{
		static_cast<VulkanAllocator*>( userData )->Free( data );
	}

	static void VKAPI_CALL InternalAllocationNotification( void* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope )
	{
		static_cast<VulkanAllocator*>( userData )->InternalAllocation( size, allocationType, scope );
	}

	static void VKAPI_CALL InternalFreeNotification( void* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope )
	{
		static_cast<VulkanAllocator*>( userData )->InternalFree( size, allocationType, scope );
	}

private:
	void* Allocate( size_t size, size_t alignment, VkSystemAllocationScope scope );

	void* Reallocate( void* original, size_t size, size_t alignment, VkSystemAllocationScope scope );

	void Free( void* data );

	void InternalAllocation( size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope );
	void InternalFree( size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope );

private:
	VkAllocationCallbacks vulkanAllocator;

	std::vector<AllocationTracker> trackers;
};
