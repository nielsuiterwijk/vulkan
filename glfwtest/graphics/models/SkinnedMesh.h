#pragma once

#include "standard.h"
#include "Mesh.h"
#include "graphics/animations/Animation.h"

#define MAX_BONES 64
#define MAX_BONES_PER_VERTEX 4

// Per-vertex bone IDs and weights
struct VertexBoneData
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> IDs;
	std::array<float, MAX_BONES_PER_VERTEX> weights;

	// Ad bone weighting to vertex info
	void add(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < MAX_BONES_PER_VERTEX; i++)
		{
			if (weights[i] == 0.0f)
			{
				IDs[i] = boneID;
				weights[i] = weight;
				return;
			}
		}
	}
};

// Stores information on a single bone
struct BoneInfo
{
	glm::mat4 offset;
	glm::mat4 finalTransformation;
};


class SkinnedMesh : public Mesh
{
	friend class MeshFileLoader;

public:
	virtual MeshType GetMeshType() const { return MeshType::Skinned; }

	void AddBone(BoneInfo boneInfo) { bones.emplace_back(boneInfo); }

private:
	std::vector<BoneInfo> bones;
};