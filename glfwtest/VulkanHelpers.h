#pragma once

#include <vulkan/vulkan.h>

#include <iostream>

namespace Vulkan
{
	//https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/
	const char* GetVendorName(int vendorId)
	{
		switch (vendorId)
		{
		case 0x1002:
			return "AMD";
		case 0x1010:
			return "ImgTec";
		case 0x10DE:
			return "NVIDIA";
		case 0x13B5:
			return "ARM";
		case 0x5143:
			return "Qualcomm";
		case 0x8086:
			return "INTEL";
		default:
			return "Unknown";
		}
	}

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT debugCallback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
		{
			func(instance, debugCallback, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	  VkDebugReportFlagsEXT flags,
	  VkDebugReportObjectTypeEXT objType,
	  uint64_t obj,
	  size_t location,
	  int32_t code,
	  const char* layerPrefix,
	  const char* msg,
	  void* userData)
	{

		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}
}