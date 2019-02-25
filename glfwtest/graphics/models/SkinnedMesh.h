#pragma once

#include "standard.h"
#include "Mesh.h"
#include "graphics/animations/Animation.h"

#include "graphics/buffers/UniformBufferDefinition.h"


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
	std::vector<int32_t> children;
	glm::mat4 offset;
	glm::mat4 finalTransformation;
};

struct SkinInfo
{
	int32_t rootBone;
	std::vector<int32_t> joints;
	std::vector<glm::mat4> inverseBindMatrices;
};


class SkinnedMesh : public Mesh
{
	friend class MeshFileLoader;

public:
	virtual MeshType GetMeshType() const { return MeshType::Skinned; }

	void Update(float delta);

	void AddBone(BoneInfo boneInfo)
	{
		bones.emplace_back(boneInfo); 
	}

	void AddSkin(SkinInfo skinInfo)
	{
		skins.emplace_back(skinInfo);
	}
	
	void SetAnimation(const std::vector<Animation>& newAnimations )
	{
		animations = newAnimations;
	}

private:

private:
	std::vector<BoneInfo> bones;
	std::vector<SkinInfo> skins;
	std::vector<Animation> animations;

	Animation* selectedAnimation = nullptr;

	SkinnedMeshBuffer ubo = {};

	float time;

	//std::vector<int32_t> boneHierarchy;
};