#pragma once

#include "graphics/Camera.h"

class FirstPersonCamera : public Camera
{
public:
	FirstPersonCamera();

	void Update();

private:
	glm::ivec2 _PrevMousePosition = { FLT_MAX, FLT_MAX };

	float Hor = 0;
	float Vert = 0;
};