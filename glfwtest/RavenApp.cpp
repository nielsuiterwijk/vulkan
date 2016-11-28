#include "RavenApp.h"

#include "standard.h"

#include <vector>
#include <iostream>

#include "DebugAssert.h"
#include "Timer.h"
#include "Helpers.h"
#include "GraphicsDevice.h"

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
	std::vector<std::string> deviceExtensionsNeeded = device->GetRequiredInstanceExtensions();

	allExtensionsRequired.insert(std::end(allExtensionsRequired), std::begin(windowExtensionsNeeded), std::end(windowExtensionsNeeded));
	allExtensionsRequired.insert(std::end(allExtensionsRequired), std::begin(deviceExtensionsNeeded), std::end(deviceExtensionsNeeded));

	for (size_t i = 0; i < allExtensionsRequired.size(); i++)
	{
		if (!device->IsExtensionAvailable(allExtensionsRequired[i]))
		{
			std::cout << "Missing " << allExtensionsRequired[i] << " extension." << std::endl;
			return false;
		}
	}

	device->CreateVulkanInstance(allExtensionsRequired);

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