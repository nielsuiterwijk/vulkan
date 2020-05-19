#include "SkinnedMesh.h"

#include "graphics/buffers/UniformBuffer.h"

#include "ecs/World.h"

namespace
{
	glm::mat4 localMatrix( const BoneInfo& bone )
	{
		return glm::translate( glm::mat4( 1.0f ), bone.translation ) * glm::toMat4( bone.rotation ) * glm::scale( glm::mat4( 1.0f ), bone.scale ) * bone.localTransform;
	}

	//Note: not very efficient calculation of the bones over and over again
	glm::mat4 CalculateMatrix( const BoneInfo& bone, const std::vector<BoneInfo>& bones ) 
	{
		glm::mat4 returnValue = localMatrix( bone );

		int32_t parentBoneIndex = bone.parent;
		while ( parentBoneIndex >= 0 )
		{
			const BoneInfo& parentBone = bones[ parentBoneIndex ];

			returnValue = localMatrix( parentBone ) * returnValue;

			parentBoneIndex = parentBone.parent;
		}

		return returnValue;
	}
}

void AnimationSystem::Tick( Ecs::World& World, float Delta )
{
	auto View = World.View<AnimationComponent, SkinnedMeshComponent>();

	for ( auto Entity : View )
	{
		AnimationComponent& AnimationData = View.Get<AnimationComponent>( Entity );
		SkinnedMeshComponent& SkinnedMeshData = View.Get<SkinnedMeshComponent>( Entity );

		if ( AnimationData._Animations.size() == 0 || SkinnedMeshData._Skins.size() == 0 )
			continue;

		constexpr float TicksPerSecond = 25.0f; //todo read from gltf
		constexpr float Duration = 2.0f; //todo read from gltf
		
		AnimationData._Time += Delta;

		while ( AnimationData._Time > Duration )
		{
			AnimationData._Time -= Duration;
		}

		float AnimationTime = AnimationData._Time; // (time / Duration)* TicksPerSecond;


		const SkinInfo& Skin = SkinnedMeshData._Skins[ 0 ];
		const Animation& Animation = AnimationData._Animations[ 0 ];
		std::vector<BoneInfo>& Bones = SkinnedMeshData._Bones;

		//Run animation
		Animation.CalculateBones( AnimationTime, Skin.rootBone, Bones );

		glm::mat4 rootTransform = CalculateMatrix( Bones[ Skin.rootBone ], Bones );
		glm::mat4 inverseRootTransform = glm::inverse( rootTransform );


		glm::mat4 jointMatrix;

		for ( int32_t i = 0; i < Skin.joints.size(); i++ )
		{
			int32_t jointIndex = Skin.joints[ i ];
			const BoneInfo& ActiveBone = Bones[ jointIndex ];

			jointMatrix = CalculateMatrix( ActiveBone, Bones ) * Skin.inverseBindMatrices[ i ];
			jointMatrix = inverseRootTransform * jointMatrix;
			SkinnedMeshData._ResultBuffer.jointMatrix[ i ] = jointMatrix;
		}

		SkinnedMeshData._ResultBuffer.model = rootTransform;
	}

}

SkinnedMesh::SkinnedMesh()
{
	//localMeshUniformBuffer = new UniformBuffer( { &skinnedMeshBuffer, sizeof( SkinnedMeshBuffer ) } );
}

SkinnedMesh::~SkinnedMesh()
{
	//Done now somewhere else (material)
	//delete localMeshUniformBuffer;
}

void SkinnedMesh::Assign( Ecs::World& World, Ecs::Entity e ) const
{
	using namespace Ecs;

	Mesh::Assign( World, e );

	World.Assign<AnimationComponent>( e, AnimationComponent { animations, nullptr, 0.0f } );
	World.Assign<SkinnedMeshComponent>( e, SkinnedMeshComponent { bones, skins } );
}


/*void SkinnedMesh::Update( float delta )
{
	if ( animations.size() == 0 || skins.size() == 0 )
		return;

	constexpr float TicksPerSecond = 25.0f; //todo read from gltf
	constexpr float Duration = 2.0f; //todo read from gltf

	const Animation& animation = animations[ 0 ];
	const SkinInfo& skin = skins[ 0 ];

	time += delta;

	while ( time > Duration )
	{
		time -= Duration;
	}

	float AnimationTime = time; // (time / Duration)* TicksPerSecond;

	//Run animation
	animation.CalculateBones( AnimationTime, skin.rootBone, bones );

	glm::mat4 rootTransform = CalculateMatrix( bones[ skin.rootBone ], bones );
	glm::mat4 inverseRootTransform = glm::inverse( rootTransform );

	skinnedMeshBuffer.model = rootTransform;

	glm::mat4 jointMatrix;

	for ( int32_t i = 0; i < skin.joints.size(); i++ )
	{
		int32_t jointIndex = skin.joints[ i ];
		const BoneInfo& bone = bones[ jointIndex ];

		jointMatrix = CalculateMatrix( bone, bones ) * skin.inverseBindMatrices[ i ];
		jointMatrix = inverseRootTransform * jointMatrix;
		skinnedMeshBuffer.jointMatrix[ i ] = jointMatrix;
	}
}*/
