#include "GraphicsDevice.h"

#include "standard.h"

#include <iostream>

GraphicsDevice::GraphicsDevice()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	availableExtensions = std::vector<VkExtensionProperties>(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::cout << extensionCount << " extensions supported" << std::endl;

	for (const VkExtensionProperties& extension : availableExtensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}


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

bool GraphicsDevice::IsExtensionAvailable(std::string extension)
{
	for (size_t i = 0; i < availableExtensions.size(); i++)
	{
		if (std::strcmp(availableExtensions[i].extensionName, extension.c_str()) == 0)
			return true;
	}

	return false;
}