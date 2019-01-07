#pragma once

#include "standard.h"

enum class Interpolation
{
	Linear,
	Step,
	CubicSpline
};

class Animation
{
public:
	std::vector<float> keyFrames = {};

	std::vector<glm::vec3> translations = {};
	std::vector<glm::vec3> scales = {};
	std::vector<glm::quat> rotations = {};

	std::string name;

	int32_t targetBone = -1;
	Interpolation interpolationMode = Interpolation::Linear;
};