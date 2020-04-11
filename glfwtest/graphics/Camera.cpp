#include "Camera.h"

void Camera::CalculateView()
{
	_Data.view = glm::toMat4(_Orientation) * glm::translate( glm::mat4( 1.0f ), -_Position );	
}

void Camera::CalculateProjection( float AngleInDegrees )
{
	_Data.proj = glm::perspective( glm::radians( AngleInDegrees ), AspectRatio, ZNear, ZFar );
}

void Camera::CalculateOrientation( float HorizontalAngleInDegrees, float VerticalAngleIndegrees )
{
	_Orientation = glm::angleAxis( glm::radians(VerticalAngleIndegrees), Camera::Right );
	_Orientation *= glm::angleAxis( glm::radians( HorizontalAngleInDegrees), Camera::Up );
}

void Camera::Translate( const glm::vec3& Direction )
{
	_Position += Direction;
}

glm::vec3 Camera::CalculateForward() const
{
	return glm::inverse( _Orientation ) * Camera::Forward; //TODO: Why inverse?
}

glm::vec3 Camera::CalculateRight() const
{
	return glm::inverse( _Orientation ) * Camera::Right;
}
