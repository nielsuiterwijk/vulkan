#pragma once

#include "standard.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/RenderObject.h"

#include "helpers/IMGUIVulkan.h"

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

public:
	static std::vector<std::function<void(int, int, int)>> OnMouseButton;
	static std::vector<std::function<void(double, double)>> OnMouseScroll;
	static std::vector<std::function<void(int, int, int, int)>> OnKey;
	static std::vector<std::function<void(unsigned int)>> OnChar;

private:
	static void UpdateThread(RavenApp* app);
	static void RenderThread(RavenApp* app); 
	
	static void OnWindowResized(GLFWwindow* window, int width, int height);

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void CharCallback(GLFWwindow* window, unsigned int c);

private:
	GLFWwindow* window;
	bool run;

	//objects..
	RenderObject* renderobject;
	std::mutex objectMutex;

	IMGUIVulkan imguiVulkan;
};