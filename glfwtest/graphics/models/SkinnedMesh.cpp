#include "SkinnedMesh.h"

void SkinnedMesh::Update(float Delta)
{
	// Update bones
/*	float TicksPerSecond = (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = time * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)scene->mAnimations[0]->mDuration);

	glm::mat4 identity = glm::mat4(	1, 0, 0, 0, 
									0, 1, 0, 0, 
									0, 0, 1, 0, 
									0, 0, 0, 1);

	readNodeHierarchy(AnimationTime, scene->mRootNode, identity);

	for (uint32_t i = 0; i < boneTransforms.size(); i++)
	{
		boneTransforms[i] = boneInfo[i].finalTransformation;
	}

	for (uint32_t i = 0; i < bones.size(); i++)
	{
		ubo.bones[i] = glm::transpose(glm::make_mat4(bones[i].finalTransformation));
	}

	uniformBuffers.mesh.copyTo(&uboVS, sizeof(uboVS));

	// Update floor animation
	uboFloor.uvOffset.t -= 0.25f * skinnedMesh->animationSpeed * frameTimer;
	uniformBuffers.floor.copyTo(&uboFloor, sizeof(uboFloor));*/
}