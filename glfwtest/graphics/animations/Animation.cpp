#include "Animation.h"

#include "graphics/models/SkinnedMesh.h"

// Get node hierarchy for current animation time
void Animation::ReadNodeHierarchy(float AnimationTime, int32_t rootBone, std::vector<BoneInfo>& bones, const glm::mat4& parentTransform) const
{
	BoneInfo& boneInfo = bones[rootBone];
	const BoneAnimation& boneAnimation = boneAnimationFrames[rootBone];

	std::string NodeName(boneAnimation.name);
	// Get interpolated matrices between current and next frame
	glm::mat4 matScale = interpolateScale(AnimationTime, boneAnimation);
	glm::mat4 matRotation = interpolateRotation(AnimationTime, boneAnimation);
	glm::mat4 matTranslation = interpolateTranslation(AnimationTime, boneAnimation);

	glm::mat4 nodeTransformation = matTranslation * matRotation * matScale;

	glm::mat4 globalTransformation = parentTransform * nodeTransformation;

	boneInfo.finalTransformation = globalTransformation * boneInfo.offset;

	for (uint32_t i = 0; i < boneInfo.children.size(); i++)
	{
		ReadNodeHierarchy(AnimationTime, boneInfo.children[i], bones, globalTransformation);
	}
}


int32_t Animation::CalculateFrame(float time, const BoneAnimation& boneAnimation) const
{
	uint32_t frameIndex = 0;
	for (uint32_t i = 0; i < boneAnimation.keyFrames.size() - 1; i++)
	{
		if (time < (float)boneAnimation.keyFrames[i + 1])
		{
			frameIndex = i;
			break;
		}
	}

	return frameIndex;
}
// Returns a 4x4 matrix with interpolated translation between current and next frame
glm::mat4 Animation::interpolateTranslation(float time, const BoneAnimation& boneAnimation) const
{
	glm::vec3 translation;

	if (boneAnimation.keyFrames.size() == 1)
	{
		translation = boneAnimation.translations[0];
	}
	else
	{
		int32_t frameIndex = CalculateFrame(time, boneAnimation);
		int32_t nextFrameIndex = (frameIndex + 1) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[frameIndex];
		float nextTime = boneAnimation.keyFrames[nextFrameIndex];

		const glm::vec3& start = boneAnimation.translations[frameIndex];
		const glm::vec3& end = boneAnimation.translations[nextFrameIndex];

		float delta = (time - startTime) / (nextTime - startTime);

		translation = glm::lerp(start, end, delta);
	}

	return glm::translate(translation);
}

glm::mat4 Animation::interpolateScale(float time, const BoneAnimation & boneAnimation) const
{
	glm::vec3 scale;

	if (boneAnimation.keyFrames.size() == 1)
	{
		scale = boneAnimation.scales[0];
	}
	else
	{
		int32_t frameIndex = CalculateFrame(time, boneAnimation);
		int32_t nextFrameIndex = (frameIndex + 1) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[frameIndex];
		float nextTime = boneAnimation.keyFrames[nextFrameIndex];

		const glm::vec3& start = boneAnimation.scales[frameIndex];
		const glm::vec3& end = boneAnimation.scales[nextFrameIndex];

		float delta = (time - startTime) / (nextTime - startTime);

		scale = glm::lerp(start, end, delta);
	}

	return glm::translate(scale);
}

// Returns a 4x4 matrix with interpolated rotation between current and next frame
glm::mat4 Animation::interpolateRotation(float time, const BoneAnimation& boneAnimation) const
{
	glm::quat rotation;

	if (boneAnimation.keyFrames.size() == 1)
	{
		rotation = boneAnimation.rotations[0];
	}
	else
	{
		int32_t frameIndex = CalculateFrame(time, boneAnimation);
		int32_t nextFrameIndex = (frameIndex + 1) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[frameIndex];
		float nextTime = boneAnimation.keyFrames[nextFrameIndex];

		const glm::quat& start = boneAnimation.rotations[frameIndex];
		const glm::quat& end = boneAnimation.rotations[nextFrameIndex];

		float delta = (time - startTime) / (nextTime - startTime);

		rotation = glm::slerp(start, end, delta);
	}

	return glm::toMat4(rotation);
}