#pragma once

class GLFWwindow;

class RavenApp
{
public:

	RavenApp();
	~RavenApp();

	void Initialize();

	void Run();

	RavenApp(const RavenApp&) = default;               // Copy constructor
	RavenApp(RavenApp&&) = default;                    // Move constructor
	RavenApp& operator=(const RavenApp&) & = default;  // Copy assignment operator
	RavenApp& operator=(RavenApp&&) & = default;       // Move assignment operator

private:
	GLFWwindow* window;

};