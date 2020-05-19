#pragma once

#include "Mesh.h"
#include "graphics/animations/Animation.h"


#include "graphics/buffers/UniformBufferDefinition.h"

#include "ecs/SystemInterface.h"
#include "ecs/Entity.h"

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


struct SkinnedMeshComponent
{
	std::vector<BoneInfo> _Bones;
	std::vector<SkinInfo> _Skins;

	SkinnedMeshBuffer _ResultBuffer = {};
};

struct AnimationComponent
{
	std::vector<Animation> _Animations;
	Animation* _pSelectedAnimation = nullptr; //Will be pointing to the object in _Animations
	float _Time;
};

class AnimationSystem : Ecs::SystemInterface
{
public:
	virtual void Tick( Ecs::World& World, float Delta ) final;
};

//TODO: Don't make this inherit from mesh, better to make a 'SkinComponent' and a 'Animation' component
class SkinnedMesh : public Mesh
{
	friend class MeshFileLoader;

public:
	virtual MeshType GetMeshType() const final { return MeshType::Skinned; }

	SkinnedMesh();
	virtual ~SkinnedMesh();
	
	virtual void Assign( Ecs::World& World, Ecs::Entity Entity ) const final;

private:
	void AddBone( BoneInfo boneInfo ) { bones.emplace_back( boneInfo ); }
	void AddSkin( SkinInfo skinInfo ) { skins.emplace_back( skinInfo ); }

	void SetAnimation( const std::vector<Animation>& newAnimations ) { animations = newAnimations; }

private:
	std::vector<BoneInfo> bones;
	std::vector<SkinInfo> skins;
	std::vector<Animation> animations;
};
