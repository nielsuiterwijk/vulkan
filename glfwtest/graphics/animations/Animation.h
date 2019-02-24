#pragma once

#include "standard.h"

enum class Interpolation
{
	Linear,
	Step,
	CubicSpline
};

class BoneAnimation
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

class Animation
{
public:
	Animation(const std::vector<BoneAnimation>& input) :
		boneAnimationFrames(input)
	{ }

	int32_t CalculateFrame(float time, const BoneAnimation& boneAnimation);
	
	// Get node hierarchy for current animation time
	void ReadNodeHierarchy(float AnimationTime, int32_t bone, const glm::mat4& parentTransform);

	// Returns a 4x4 matrix with interpolated translation between current and next frame
	glm::mat4 interpolateTranslation(float time, const BoneAnimation& boneAnimation);
	glm::mat4 interpolateScale(float time, const BoneAnimation& boneAnimation);

	// Returns a 4x4 matrix with interpolated rotation between current and next frame
	glm::mat4 interpolateRotation(float time, const BoneAnimation& boneAnimation);

	std::vector<BoneAnimation> boneAnimationFrames = {};
};
