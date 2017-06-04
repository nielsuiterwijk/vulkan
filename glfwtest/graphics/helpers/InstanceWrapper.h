#pragma once

#include <functional>

#include <vulkan/vulkan.h>
#include "Allocator.h"

//The idea behind this class is that if you delete or overwrite the object, it will always call the correct vulkan clean up code
//to prevent any memory leaks.

//TODO: Add reference counting.. probably a good idea :)
template <typename T>
class InstanceWrapper
{
public:
	InstanceWrapper() : InstanceWrapper([](T, const VkAllocationCallbacks*) {}, nullptr) {}

	InstanceWrapper(std::function<void(T, const VkAllocationCallbacks*)> callback, const VkAllocationCallbacks* allocator) :
		allocationCallbacks(allocator)
	{
		this->deleteCallback = [ = ](T obj)
		{
			callback(obj, allocator);
		};
	}

	InstanceWrapper(const InstanceWrapper<VkInstance>& instance, std::function<void(VkInstance, T, const VkAllocationCallbacks*)> callback, const VkAllocationCallbacks* allocator) :
		allocationCallbacks(allocator)
	{
		this->deleteCallback = [this, &instance, callback, allocator](T obj)
		{
			callback(instance, obj, allocator);
		};
	}

	InstanceWrapper(const InstanceWrapper<VkDevice>& device, std::function<void(VkDevice, T, const VkAllocationCallbacks*)> callback, const VkAllocationCallbacks* allocator) :
		allocationCallbacks(allocator)
	{
		this->deleteCallback = [this, &device, callback, allocator](T obj)
		{
			callback(device, obj, allocator);
		};
	}

	~InstanceWrapper()
	{
		Cleanup();
		deleteCallback = nullptr;
		allocationCallbacks = nullptr;
	}

	//This is used in case of having a std::vector of InstanceWrappers. The vector calls the default constructor
	void Initialize(const InstanceWrapper<VkDevice>& device, std::function<void(VkDevice, T, const VkAllocationCallbacks*)> callback, const VkAllocationCallbacks* allocator)
	{
		{
			Cleanup();
			deleteCallback = nullptr;
			allocationCallbacks = nullptr;
		}

		allocationCallbacks = allocator;

		this->deleteCallback = [this, &device, callback, allocator](T obj)
		{
			callback(device, obj, allocator);
		};
	}

	const T* operator &() const
	{
		return &vulkanObject;
	}

	T* Replace()
	{
		Cleanup();
		return &vulkanObject;
	}

	const VkAllocationCallbacks* AllocationCallbacks()
	{
		return allocationCallbacks;
	}

	operator T() const
	{
		return vulkanObject;
	}

	void operator=(T rhs)
	{
		if (rhs != vulkanObject)
		{
			Cleanup();
			vulkanObject = rhs;
		}
	}

	template<typename V>
	bool operator==(V rhs)
	{
		return vulkanObject == T(rhs);
	}

private:
	void Cleanup()
	{
		if (vulkanObject != VK_NULL_HANDLE)
		{
			deleteCallback(vulkanObject);
		}

		vulkanObject = VK_NULL_HANDLE;
	}

private:
	T vulkanObject { VK_NULL_HANDLE };

	std::function<void(T)> deleteCallback;
	const VkAllocationCallbacks* allocationCallbacks;
};