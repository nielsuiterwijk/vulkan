#include "SkinnedMesh.h"
#include "graphics/buffers/UniformBuffer.h"

SkinnedMesh::SkinnedMesh()
{
	ubo = new UniformBuffer({ &skinnedMeshBuffer, sizeof(SkinnedMeshBuffer) });
}

SkinnedMesh::~SkinnedMesh()
{
	delete ubo;
}

void SkinnedMesh::Update(float delta)
{
	if (animations.size() == 0 || skins.size() == 0)
		return;

	const Animation & animation = animations[0];
	const SkinInfo & skin = skins[0];

	time += delta;

	constexpr float TicksPerSecond = 25.0f; //todo read from gltf
	constexpr float Duration = 2.0f; //todo read from gltf
	float TimeInTicks = time * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, Duration); //todo read from gltf

	glm::mat4 identity = {};

	animation.ReadNodeHierarchy(AnimationTime, skin.rootBone, bones, identity);
	
	/*jointMatrix(j) = globalTransformOfNodeThatTheMeshIsAttachedTo^-1 * globalTransformOfJointNode(j) * inverseBindMatrixForJoint(j);*/
	for (uint32_t i = 0; i < bones.size(); i++)
	{
		skinnedMeshBuffer.bones[i] = bones[i].finalTransformation;
	}
}