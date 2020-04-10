#pragma once

#include <functional>

#include "graphics/memory/VulkanAllocator.h"
#include <vulkan/vulkan.h>

//The idea behind this class is that if you delete or overwrite the object, it will always call the correct vulkan clean up code
//to prevent any memory leaks.
//Works similar to a std::unique_ptr 

template <typename T>
class InstanceWrapper
{
public:
	InstanceWrapper()
		: InstanceWrapper( []( T, const VkAllocationCallbacks* ) {}, nullptr )
	{
	}

	InstanceWrapper( std::function<void( T, const VkAllocationCallbacks* )> callback, const VkAllocationCallbacks* allocator )
		: _AllocationCallbacks( allocator )
	{
		this->_DeleteCallback = [ = ]( T obj ) {
			std::cout << "Deleting InstanceWrapper" << std::endl;
			callback( obj, allocator );
		};
	}

	InstanceWrapper( const InstanceWrapper<VkInstance>& instance, std::function<void( VkInstance, T, const VkAllocationCallbacks* )> callback, const VkAllocationCallbacks* allocator )
		: _AllocationCallbacks( allocator )
	{
		this->_DeleteCallback = [ this, &instance, callback, allocator ]( T obj ) {
			std::cout << "Deleting InstanceWrapper" << std::endl;
			callback( instance, obj, allocator );
		};
	}

	InstanceWrapper( const InstanceWrapper<VkDevice>& device, std::function<void( VkDevice, T, const VkAllocationCallbacks* )> callback, const VkAllocationCallbacks* allocator )
		: _AllocationCallbacks( allocator )
	{
		this->_DeleteCallback = [ this, &device, callback, allocator ]( T obj ) {
			std::cout << "Deleting InstanceWrapper" << std::endl;
			callback( device, obj, allocator );
		};
	}

	~InstanceWrapper()
	{
		Cleanup();
		_DeleteCallback = nullptr;
		_AllocationCallbacks = nullptr;
	}

	//This is used in case of having a std::vector of InstanceWrappers. The vector calls the default constructor
	void Initialize( const InstanceWrapper<VkDevice>& device, std::function<void( VkDevice, T, const VkAllocationCallbacks* )> callback, const VkAllocationCallbacks* allocator )
	{
		{
			Cleanup();
			_DeleteCallback = nullptr;
			_AllocationCallbacks = nullptr;
		}

		_AllocationCallbacks = allocator;

		this->_DeleteCallback = [ this, &device, callback, allocator ]( T obj ) {
			std::cout << "Deleting InstanceWrapper" << std::endl;
			callback( device, obj, allocator );
		};
	}

	const T* operator&() const { return &_VulkanObject; }

	operator T() const { return _VulkanObject; }

	T* Replace()
	{
		Cleanup();
		return &_VulkanObject;
	}

	const VkAllocationCallbacks* AllocationCallbacks() { return _AllocationCallbacks; }

	void operator=( T rhs )
	{
		if ( rhs != _VulkanObject )
		{
			Cleanup();
			_VulkanObject = rhs;
		}
	}

	template <typename V>
	bool operator==( V rhs )
	{
		return _VulkanObject == T( rhs );
	}

private:
	void Cleanup()
	{
		if ( _VulkanObject != VK_NULL_HANDLE )
		{
			_DeleteCallback( _VulkanObject );
		}

		_VulkanObject = VK_NULL_HANDLE;
	}

private:
	T _VulkanObject { VK_NULL_HANDLE };

	std::function<void( T )> _DeleteCallback;
	const VkAllocationCallbacks* _AllocationCallbacks;
};
