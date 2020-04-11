#pragma once


class InputEvent
{
public:
	static std::vector<std::function<void( int, int, int )>> OnMouseButton;
	static std::vector<std::function<void( double, double )>> OnMouseScroll;
	static std::vector<std::function<void( int, int, int, int )>> OnKey;
	static std::vector<std::function<void( unsigned int )>> OnChar;


public:

	static void InputEvent::MouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
	{
		for ( size_t i = 0; i < OnMouseButton.size(); i++ )
		{
			OnMouseButton[ i ]( button, action, mods );
		}
	}

	static void InputEvent::ScrollCallback( GLFWwindow* window, double xoffset, double yoffset )
	{
		for ( size_t i = 0; i < OnMouseScroll.size(); i++ )
		{
			OnMouseScroll[ i ]( xoffset, yoffset );
		}
	}

	static void InputEvent::KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
	{
		for ( size_t i = 0; i < OnKey.size(); i++ )
		{
			OnKey[ i ]( key, scancode, action, mods );
		}
	}

	static void InputEvent::CharCallback( GLFWwindow* window, unsigned int c )
	{
		for ( size_t i = 0; i < OnChar.size(); i++ )
		{
			OnChar[ i ]( c );
		}
	}

};