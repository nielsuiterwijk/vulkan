#pragma once

#include "Mesh.h"
#include "graphics/animations/Animation.h"


#include "graphics/buffers/UniformBufferDefinition.h"

class UniformBuffer;

// Stores information on a single bone
struct BoneInfo
{
	int32_t jointIndex = -1;
	int32_t parent = -1;
	std::vector<int32_t> children = {};

	glm::mat4 localTransform = {};

	//animation
	glm::vec3 translation = {};
	glm::vec3 scale = {};
	glm::quat rotation = {};
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

	void Update( float delta );

	void AddBone( BoneInfo boneInfo ) { bones.emplace_back( boneInfo ); }
	void AddSkin( SkinInfo skinInfo ) { skins.emplace_back( skinInfo ); }

	void SetAnimation( const std::vector<Animation>& newAnimations ) { animations = newAnimations; }

	UniformBuffer* AccessUBO() { return localMeshUniformBuffer; }

private:
	glm::mat4 localMatrix( const BoneInfo& bone );
	glm::mat4 getMatrix( const BoneInfo& bone );

private:
	std::vector<BoneInfo> bones;
	std::vector<SkinInfo> skins;
	std::vector<Animation> animations;

	Animation* selectedAnimation = nullptr;

	SkinnedMeshBuffer skinnedMeshBuffer = {};
	UniformBuffer* localMeshUniformBuffer = nullptr;

	float time = 0;
};
