#include "RavenApp.h"

#include "standard.h"

#include <vector>
#include <iostream>

#include "helpers/DebugAssert.h"
#include "helpers/Timer.h"
#include "helpers/Helpers.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/VulkanInstance.h"
#include "graphics\VulkanSwapChain.h"

RavenApp::RavenApp() :
	window(nullptr),
	device(nullptr)
{

}

RavenApp::~RavenApp()
{
	delete device;
	device = nullptr;

	glfwDestroyWindow(window);

	glfwTerminate();
}

bool RavenApp::Initialize()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);

	device = new GraphicsDevice(glm::u32vec2(1280, 720));

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<std::string> allExtensionsRequired;

	std::vector<std::string> windowExtensionsNeeded(glfwExtensions, glfwExtensions + glfwExtensionCount);

	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanInstance> vulkanInstance = std::make_shared<VulkanInstance>();

	for (size_t i = 0; i < allExtensionsRequired.size(); i++)
	{
		if (!vulkanInstance->IsExtensionAvailable(allExtensionsRequired[i]))
		{
			std::cout << "Missing " << allExtensionsRequired[i] << " extension." << std::endl;
			return false;
		}
	}

	vulkanInstance->CreateInstance(windowExtensionsNeeded);
	vulkanInstance->HookDebugCallback();

	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>(vulkanInstance->Get());

	if (glfwCreateWindowSurface(vulkanInstance->Get(), window, &((VkAllocationCallbacks)vulkanSwapChain->GetSurface().AllocationCallbacks()), vulkanSwapChain->GetSurface().Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	device->Initialize(vulkanInstance, vulkanSwapChain);

	return true;
}

void RavenApp::Run()
{
	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	Timer timer;

	while (!glfwWindowShouldClose(window))
	{
		timer.Start();

		glfwPollEvents();

		Sleep(5);

		timer.Stop();

		std::string windowTitle = std::string("delta time: ") + Helpers::ValueToString(timer.GetTimeInSeconds()*1000.0f);

		glfwSetWindowTitle(window, windowTitle.c_str());
	}
}