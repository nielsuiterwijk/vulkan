#pragma once

#include <vulkan/vulkan.h>

#include <iostream>

class Vulkan
{
public:
	//https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/
	static const char* GetVendorName(int vendorId)
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
};