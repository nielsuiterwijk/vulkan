#pragma once

#include "standard.h"
#include "graphics/GraphicsDevice.h"

struct GLFWwindow;

class RavenApp
{
public:

	RavenApp();
	~RavenApp();

	bool Initialize();

	void Run();

	RavenApp(const RavenApp&) = default;               // Copy constructor
	RavenApp(RavenApp&&) = default;                    // Move constructor
	RavenApp& operator=(const RavenApp&) & = default;  // Copy assignment operator
	RavenApp& operator=(RavenApp&&) & = default;       // Move assignment operator

private:
	static void UpdateThread(RavenApp& app);
	static void RenderThread(RavenApp& app); 
	
	static void OnWindowResized(GLFWwindow* window, int width, int height)
	{
		if (width == 0 || height == 0)
			return;

		GraphicsContext::WindowSize = glm::ivec2(width, height);

		GraphicsDevice::Instance().SwapchainInvalidated();

		RavenApp* app = reinterpret_cast<RavenApp*>(glfwGetWindowUserPointer(window));
	}

private:
	GLFWwindow* window;
	bool run;
};