#pragma once

#include <functional>

#include <vulkan/vulkan.h>
#include "Allocator.h"

//The idea behind this class is that if you delete or overwrite the object, it will always call the correct vulkan clean up code
//to prevent any memory leaks.
//TODO: Add the Allocator code to this call
template <typename T>
class InstanceWrapper
{
public:
	InstanceWrapper() : InstanceWrapper([](T, VkAllocationCallbacks*) {}) {}

	InstanceWrapper(std::function<void(T, VkAllocationCallbacks*)> callback)
	{
		this->deleteCallback = [ = ](T obj)
		{
			callback(obj, &((VkAllocationCallbacks)allocator));
		};
	}

	InstanceWrapper(const InstanceWrapper<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> callback)
	{
		this->deleteCallback = [this, &instance, callback](T obj)
		{
			callback(instance, obj, &((VkAllocationCallbacks)allocator));
		};
	}

	InstanceWrapper(const InstanceWrapper<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> callback)
	{
		this->deleteCallback = [this, &device, callback](T obj)
		{
			callback(device, obj, &((VkAllocationCallbacks)allocator));
		};
	}

	~InstanceWrapper()
	{
		Cleanup();
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

	const Allocator& AllocationCallbacks()
	{
		return allocator;
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

	Allocator allocator;

};