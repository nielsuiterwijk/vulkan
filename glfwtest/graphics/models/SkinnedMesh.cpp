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
	//return (bone.translation * bone.rotation * bone.scale) * bone.localTransform;
	return glm::translate(glm::mat4(1.0f), bone.translation) * glm::toMat4(bone.rotation) * glm::scale(glm::mat4(1.0f), bone.scale) * bone.localTransform;


	//glm::vec3 position = glm::vec3(0.0f);// node.translation.size() == 0 ? glm::vec3(0.0f) : glm::make_vec3(node.translation.data());
	//glm::vec3 scale = glm::vec3(1.0f);// node.scale.size() == 0 ? glm::vec3(1.0f) : glm::make_vec3(node.scale.data());
	//glm::quat rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);// node.rotation.size() == 0 ? glm::vec4(0.0f) : glm::make_vec4(node.rotation.data());
	//
	//return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

	//return glm::mat4() * bone.localTransform;
}

glm::mat4 SkinnedMesh::getMatrix(const BoneInfo& bone)
{
	glm::mat4 returnValue = localMatrix(bone);
	//glm::mat4 m = localMatrix();

	int32_t parentBoneIndex = bone.parent;
	while (parentBoneIndex >= 0)
	{
		const BoneInfo& parentBone = bones[parentBoneIndex];
		//vkglTF::Node *p = parent;
		returnValue = localMatrix(parentBone) * returnValue;
		//m = p->localMatrix() * m;
		parentBoneIndex = parentBone.parent;
		//p = p->parent;
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
	float TimeInTicks = time* TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, Duration); //todo read from gltf

	glm::mat4 identity = {};
	
	animation.ReadNodeHierarchy(AnimationTime, skin.rootBone, bones, identity);


	glm::mat4 rootTransform = getMatrix( bones[skin.rootBone] );
	glm::mat4 inverseRootTransform = glm::inverse(rootTransform);

	skinnedMeshBuffer.model = rootTransform;

	/*for (size_t i = 0; i < numJoints; i++) {
		vkglTF::Node *jointNode = skin->joints[i];*/
	//= vkglTF::Node* node = nodeFromIndex(jointIndex);
	//i = mapped to skin.joints

	glm::mat4 jointMatrix;
	for (int32_t i = 0; i < skin.joints.size(); i++)
	{
		int32_t jointIndex = skin.joints[i];
		const BoneInfo& bone = bones[jointIndex];
		//vkglTF::Node *jointNode = skin->joints[i];

		/*jointMatrix(j) = globalTransformOfNodeThatTheMeshIsAttachedTo^-1 * globalTransformOfJointNode(j) * inverseBindMatrixForJoint(j);*/
  		jointMatrix = getMatrix(bone) * skin.inverseBindMatrices[i];
		//glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
		jointMatrix = inverseRootTransform * jointMatrix;
		//jointMat = inverseTransform * jointMat;

		//if (i < 3)
		{
			skinnedMeshBuffer.jointMatrix[i] = jointMatrix;// rootTransform* inverseRootTransform;
		}
		//else 
		//{
		//	skinnedMeshBuffer.jointMatrix[i] = glm::mat4(1.0);
		//}
		//mesh->uniformBlock.jointMatrix[i] = jointMat;

		//skinnedMeshBuffer.jointMatrix[jointIndex] = getMatrix(bone) * skin.inverseBindMatrices[a];
	}
}