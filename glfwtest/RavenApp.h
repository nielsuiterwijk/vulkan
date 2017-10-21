#pragma once

#include "standard.h"

struct GLFWwindow;
class GraphicsDevice;

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

private:
	GLFWwindow* window;
	bool run;

	std::shared_ptr<GraphicsDevice> device;
};