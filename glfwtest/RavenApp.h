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
	GLFWwindow* window;

	std::shared_ptr<GraphicsDevice> device;
};