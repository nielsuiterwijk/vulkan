#pragma once

#include "graphics/GraphicsDevice.h"
#include "graphics/RenderThread.h"
#include "helpers/IMGUIVulkan.h"
#include "helpers/Timer.h"
#include "io/InputEvent.h"

struct GLFWwindow;
class Mesh;
class GameObject;

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
	static std::vector<std::function<void( int, int )>> OnWindowResized;

private:
	static void WindowResizedCallback( GLFWwindow* window, int width, int height );

	static void MouseButtonCallback( GLFWwindow* window, int button, int action, int mods );
	static void ScrollCallback( GLFWwindow* window, double xoffset, double yoffset );
	static void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );
	static void CharCallback( GLFWwindow* window, unsigned int c );

	static void Render( RavenApp* app );

private:
	GLFWwindow* _pWindow;
	bool _Run;

	uint64_t updateFrameIndex;

	//objects..
	std::vector<std::shared_ptr<GameObject>> _GameObjects;

	IMGUIVulkan* _pImguiVulkan = nullptr;

	InputEvent _InputEvent;
	CRenderThread _RenderThread;
};
