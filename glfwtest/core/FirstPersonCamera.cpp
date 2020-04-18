#include "FirstPersonCamera.h"

#include "io/InputEvent.h"

FirstPersonCamera::FirstPersonCamera()
{
	CalculateProjection( 45.0f );

	_Position = { 2.72741079, 1.29396248, 2.77101159 };
	_Horizontal = -43.3333168;
	_Vertical = 15.499;
	CalculateOrientation( _Horizontal, _Vertical );

	CalculateView();
}

void FirstPersonCamera::Update()
{
	if ( _PrevMousePosition.x > 10000 )
	{
		_PrevMousePosition = InputEvent::MousePosition();
		return;
	}
	glm::ivec2 Delta = InputEvent::MousePosition() - _PrevMousePosition;

	constexpr glm::vec2 Speed = { 1 / 6.0f, 1 / 6.0f };

	if ( InputEvent::IsMouseButtonDown( MouseButton::Left ) )
	{
		_Horizontal += Delta.x * Speed.x;
		_Vertical += Delta.y * Speed.y;
		//Rotate( Delta.x * Speed.x, Camera::Up ); //Yaw
		//Rotate( Delta.y * Speed.y, Camera::Right ); //Pitch
		CalculateOrientation( _Horizontal, _Vertical );
	}

	if ( InputEvent::IsKeyDown( 'S' ) )
	{
		_Position -= CalculateForward() * 0.1f;
	}
	else if ( InputEvent::IsKeyDown( 'W' ) )
	{
		_Position += CalculateForward() * 0.1f;
	}
	if ( InputEvent::IsKeyDown( 'A' ) )
	{
		_Position -= CalculateRight() * 0.1f;
	}
	else if ( InputEvent::IsKeyDown( 'D' ) )
	{
		_Position += CalculateRight() * 0.1f;
	}

	CalculateView();
	_PrevMousePosition = InputEvent::MousePosition();
}