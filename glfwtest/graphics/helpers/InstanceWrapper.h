#pragma once

#include <functional>

#include <vulkan/vulkan.h>

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
			callback(obj, nullptr);
		};
	}

	InstanceWrapper(const InstanceWrapper<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> callback)
	{
		this->deleteCallback = [&instance, callback](T obj)
		{
			callback(instance, obj, nullptr);
		};
	}

	InstanceWrapper(const InstanceWrapper<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> callback)
	{
		this->deleteCallback = [&device, callback](T obj)
		{
			callback(device, obj, nullptr);
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
	T vulkanObject { VK_NULL_HANDLE };

	std::function<void(T)> deleteCallback;

	void Cleanup()
	{
		if (vulkanObject != VK_NULL_HANDLE)
		{
			deleteCallback(vulkanObject);
		}

		vulkanObject = VK_NULL_HANDLE;
	}
};