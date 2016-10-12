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

void RavenApp::Initialize()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


	window = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << extensionCount << " extensions supported" << std::endl;

	for (const auto& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	device = new GraphicsDevice();
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