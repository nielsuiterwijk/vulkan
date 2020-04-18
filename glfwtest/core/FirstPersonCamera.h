#pragma once

#include "graphics/Camera.h"

class FirstPersonCamera : public Camera
{
public:
	FirstPersonCamera();

	void Update();

private:
	glm::ivec2 _PrevMousePosition = { FLT_MAX, FLT_MAX };

	float _Horizontal = 0;
	float _Vertical = 0;
};