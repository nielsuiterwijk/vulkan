#include "FirstPersonCamera.h"

#include "io/InputEvent.h"

FirstPersonCamera::FirstPersonCamera()
{
	CalculateProjection( 45.0f );
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
		Hor += Delta.x * Speed.x;
		Vert += Delta.y * Speed.y;
		//Rotate( Delta.x * Speed.x, Camera::Up ); //Yaw
		//Rotate( Delta.y * Speed.y, Camera::Right ); //Pitch
		CalculateOrientation( Hor, Vert );
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