#include "SkinnedMesh.h"
#include "graphics/buffers/UniformBuffer.h"

SkinnedMesh::SkinnedMesh()
{
	localMeshUniformBuffer = new UniformBuffer({ &skinnedMeshBuffer, sizeof(SkinnedMeshBuffer) });
}

SkinnedMesh::~SkinnedMesh()
{
	delete localMeshUniformBuffer;
}


glm::mat4 SkinnedMesh::localMatrix(const BoneInfo& bone)
{
	return bone.matTranslation * bone.matRotation * bone.matScale * bone.localTransform;
}

glm::mat4 SkinnedMesh::getMatrix(const BoneInfo& bone)
{
	int32_t parentBoneIndex = bone.parent;
	glm::mat4 returnValue = localMatrix(bone);
	
	while (parentBoneIndex >= 0)
	{
		const BoneInfo& parentBone = bones[parentBoneIndex];
		returnValue = localMatrix(parentBone) * returnValue;
		parentBoneIndex = parentBone.parent;
	}
	return returnValue;
}


void SkinnedMesh::Update(float delta)
{
	if (animations.size() == 0 || skins.size() == 0)
		return;

	const Animation& animation = animations[0];
	const SkinInfo& skin = skins[0];

	time += delta;

	constexpr float TicksPerSecond = 25.0f; //todo read from gltf
	constexpr float Duration = 2.0f; //todo read from gltf
	float TimeInTicks = time * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, Duration); //todo read from gltf

	glm::mat4 identity = {};
	
	animation.ReadNodeHierarchy(AnimationTime, skin.rootBone, bones, identity);


	glm::mat4 rootTransform = getMatrix( bones[skin.rootBone] );
	glm::mat4 inverseRootTransform = glm::inverse(rootTransform);

	glm::mat4 jointMatrix;
	int32_t a = 0;
	for (int32_t jointIndex : skin.joints)
	{
		const BoneInfo& bone = bones[jointIndex];

		/*jointMatrix(j) = globalTransformOfNodeThatTheMeshIsAttachedTo^-1 * globalTransformOfJointNode(j) * inverseBindMatrixForJoint(j);*/
		jointMatrix = inverseRootTransform * getMatrix(bone) * skin.inverseBindMatrices[a];

		skinnedMeshBuffer.jointMatrix[jointIndex] = rootTransform * inverseRootTransform;
		a++;
	}
}