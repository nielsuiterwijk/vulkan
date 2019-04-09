#pragma once

#include "standard.h"
#include "Mesh.h"
#include "graphics/animations/Animation.h"

#include "graphics/buffers/UniformBufferDefinition.h"

class UniformBuffer;

// Per-vertex bone IDs and weights
struct VertexBoneData
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> IDs {};
	std::array<float, MAX_BONES_PER_VERTEX> weights {};
	int idx = 0;

	// Add bone weighting to vertex info
	void Add(uint32_t boneID, float weight)
	{
		if (idx >= MAX_BONES_PER_VERTEX)
			return;

		IDs[idx] = boneID;
		weights[idx] = weight;
		idx++;
	}
};

// Stores information on a single bone
struct BoneInfo
{
	int32_t index = -1;
	int32_t parent = -1;
	std::vector<int32_t> children = {};

	glm::mat4 localTransform = {};
	glm::mat4 finalTransformation = {};
	//glm::mat4 inverseBindMatrix = {};

	//animation
	glm::mat4 matScale = {};
	glm::mat4 matRotation = {};
	glm::mat4 matTranslation = {};
};

struct SkinInfo
{
	std::string name = {};
	int32_t rootBone = {};
	std::vector<int32_t> joints = {};
	std::vector<glm::mat4> inverseBindMatrices = {};
};


class SkinnedMesh : public Mesh
{
	friend class MeshFileLoader;

public:	
	virtual MeshType GetMeshType() const { return MeshType::Skinned; }

	SkinnedMesh();
	virtual ~SkinnedMesh();

	void Update(float delta);

	void AddBone(BoneInfo boneInfo) { bones.emplace_back(boneInfo); }
	void AddSkin(SkinInfo skinInfo) { skins.emplace_back(skinInfo); }
	
	void SetAnimation(const std::vector<Animation>& newAnimations ) { animations = newAnimations; }

	UniformBuffer* AccessUBO() { return localMeshUniformBuffer; }

private:
	glm::mat4 localMatrix(const BoneInfo& bone);
	glm::mat4 getMatrix(const BoneInfo& bone);


private:
	std::vector<BoneInfo> bones;
	std::vector<SkinInfo> skins;
	std::vector<Animation> animations;

	Animation* selectedAnimation = nullptr;

	SkinnedMeshBuffer skinnedMeshBuffer = {};
	UniformBuffer* localMeshUniformBuffer = nullptr;

	float time = 0;

	//std::vector<int32_t> boneHierarchy;
};