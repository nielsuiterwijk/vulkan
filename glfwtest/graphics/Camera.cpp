#include "Camera.h"


void Camera::UpdateBuffer()
{
	_Data.view = glm::lookAt( _Position, _Position + _Direction, glm::vec3( 0, 1, 0 ) );
}

void Camera::Rotate( float Amount, glm::vec3& Axis )
{
	constexpr float Speed = 0.314f;
	_Direction = glm::rotate( _Direction, Amount * Speed, Axis );
}

void Camera::Translate( glm::vec3& Direction )
{
	_Position += Direction;
}