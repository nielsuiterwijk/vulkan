#pragma once

#include "standard.h"

class Animation
{
public:
	std::vector<float> keyFrames;

	std::vector<glm::vec3> translations;
	std::vector<glm::vec3> scales;
	std::vector<glm::quat> rotations;
};