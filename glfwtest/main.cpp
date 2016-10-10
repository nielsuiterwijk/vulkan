#include "Integer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DebugAssert.h"
#include "Timer.h"
#include "Helpers.h"

#include <iostream>

int main() 
{
	Integer::RunSanityCheck();

	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported" << std::endl;

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

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}