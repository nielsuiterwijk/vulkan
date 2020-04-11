#include "InputEvent.h"


std::vector<std::function<void( int, int, int )>> InputEvent::OnMouseButton = {};
std::vector<std::function<void( double, double )>> InputEvent::OnMouseScroll = {};
std::vector<std::function<void( int, int, int, int )>> InputEvent::OnKey = {};
std::vector<std::function<void( unsigned int )>> InputEvent::OnChar = {};

static glm::dvec2 CurrentMousePosition = {};

static std::array<bool, 3> MouseButtonState = {};
static std::array<int, 256> KeyState = {};

glm::ivec2 InputEvent::MousePosition()
{
	return CurrentMousePosition;
}

bool InputEvent::IsMouseButtonDown( MouseButton Button )
{
	return MouseButtonState[ (int32_t)Button ];
}

bool InputEvent::IsKeyDown( int32_t Key )
{
	return KeyState[ (unsigned char)Key ];
}

void InputEvent::Update( GLFWwindow* pWindow, float DeltaTime )
{
	glfwGetCursorPos( pWindow, &CurrentMousePosition.x, &CurrentMousePosition.y );
}


void InputEvent::MouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
{
	for ( size_t i = 0; i < OnMouseButton.size(); i++ )
	{
		OnMouseButton[ i ]( button, action, mods );
	}

	MouseButtonState[ button ] = action == GLFW_PRESS;
}

void InputEvent::ScrollCallback( GLFWwindow* window, double xoffset, double yoffset )
{
	for ( size_t i = 0; i < OnMouseScroll.size(); i++ )
	{
		OnMouseScroll[ i ]( xoffset, yoffset );
	}
}

void InputEvent::KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	for ( size_t i = 0; i < OnKey.size(); i++ )
	{
		OnKey[ i ]( key, scancode, action, mods );
	}

	KeyState[ (unsigned char)key ] = action != GLFW_RELEASE;
}

void InputEvent::CharCallback( GLFWwindow* window, unsigned int c )
{
	for ( size_t i = 0; i < OnChar.size(); i++ )
	{
		OnChar[ i ]( c );
	}
}