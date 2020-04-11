#pragma once

/*
	The way this class works, it provides static functions to get direct input stuff. 
	But it requires an instance of it somewhere that can update the underlying static data.
*/

enum class MouseButton
{
	Left,
	Right,
	Middle
};

class InputEvent
{
public:
	static std::vector<std::function<void( int, int, int )>> OnMouseButton;
	static std::vector<std::function<void( double, double )>> OnMouseScroll;
	static std::vector<std::function<void( int, int, int, int )>> OnKey;
	static std::vector<std::function<void( unsigned int )>> OnChar;


	static glm::ivec2 MousePosition();
	static bool IsMouseButtonDown( MouseButton Button );
	static bool IsKeyDown( int32_t Key );

public:
	void Update( GLFWwindow* pWindow, float DeltaTime );
public:

	static void MouseButtonCallback( GLFWwindow* window, int button, int action, int mods );

	static void ScrollCallback( GLFWwindow* window, double xoffset, double yoffset );

	static void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods );

	static void CharCallback( GLFWwindow* window, unsigned int c );
};