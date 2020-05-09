#pragma once



struct BoneInfo;

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
	Animation( const std::vector<BoneAnimation>& input ) :
		boneAnimationFrames( input )
	{
	}

	// Get node hierarchy for current animation time
	void CalculateBones( float AnimationTime, int32_t rootBone, std::vector<BoneInfo>& bones ) const;

	std::vector<BoneAnimation> boneAnimationFrames = {};

private:
	inline int32_t CalculateFrame( float time, const BoneAnimation& boneAnimation ) const;

	// Returns a 4x4 matrix with interpolated translation between current and next frame
	glm::vec3 interpolateTranslation( float time, const BoneAnimation& boneAnimation ) const;
	glm::vec3 interpolateScale( float time, const BoneAnimation& boneAnimation ) const;
	glm::quat interpolateRotation( float time, const BoneAnimation& boneAnimation ) const;
};
