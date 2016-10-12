#include "GraphicsDevice.h"

#include "standard.h"

#include <iostream>

GraphicsDevice::GraphicsDevice()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Raven Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Raven";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, applicationInfo.Replace());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

}

GraphicsDevice::~GraphicsDevice()
{

}