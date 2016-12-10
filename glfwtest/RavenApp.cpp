#include "RavenApp.h"

#include "standard.h"

#include <vector>
#include <iostream>

#include "DebugAssert.h"
#include "Timer.h"
#include "Helpers.h"
#include "GraphicsDevice.h"
#include "VulkanRenderer.h"

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

	device = new GraphicsDevice();

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<std::string> allExtensionsRequired;

	std::vector<std::string> windowExtensionsNeeded(glfwExtensions, glfwExtensions + glfwExtensionCount);

	std::shared_ptr<VulkanRenderer> vulkanRenderer = std::make_shared<VulkanRenderer>();

	for (size_t i = 0; i < allExtensionsRequired.size(); i++)
	{
		if (!vulkanRenderer->IsExtensionAvailable(allExtensionsRequired[i]))
		{
			std::cout << "Missing " << allExtensionsRequired[i] << " extension." << std::endl;
			return false;
		}
	}

	vulkanRenderer->CreateInstance(windowExtensionsNeeded);


	vulkanRenderer->HookDebugCallback();

	if (glfwCreateWindowSurface(vulkanRenderer->GetInstance(), window, nullptr, vulkanRenderer->GetSurface().Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	device->Initialize(vulkanRenderer);

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