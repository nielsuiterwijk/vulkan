#pragma once

#include "graphics/GraphicsDevice.h"

#include "helpers/IMGUIVulkan.h"
#include "helpers/Timer.h"

#include "graphics/RenderThread.h"

struct GLFWwindow;
class Mesh;
class Model;

class RavenApp
{
public:
	RavenApp();
	~RavenApp();

	bool Initialize();

	void Run();

	RavenApp( const RavenApp& ) = default; // Copy constructor
	RavenApp( RavenApp&& ) = default; // Move constructor
	RavenApp& operator=( const RavenApp& ) & = default; // Copy assignment operator
	RavenApp& operator=( RavenApp&& ) & = default; // Move assignment operator

public:
	static std::vector<std::function<void( int, int, int )>> OnMouseButton;
	static std::vector<std::function<void( double, double )>> OnMouseScroll;
	static std::vector<std::function<void( int, int, int, int )>> OnKey;
	static std::vector<std::function<void( unsigned int )>> OnChar;

	static std::vector<std::function<void( int, int )>> OnWindowResized;

private:
	static void RenderThread( RavenApp* app );

	static void WindowResizedCallback( GLFWwindow* window, int width, int height );

	static void MouseButtonCallback( GLFWwindow* window, int button, int action, int mods );
	static void ScrollCallback( GLFWwindow* window, double xoffset, double yoffset );
	static void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
	static void CharCallback( GLFWwindow* window, unsigned int c );

	static void Render( RavenApp* app );

private:
	GLFWwindow* window;
	bool run;

	uint64_t updateFrameIndex;

	//objects..
	std::vector<std::shared_ptr<Model>> models;

	IMGUIVulkan* imguiVulkan;

	CRenderThread _RenderThread;
};
