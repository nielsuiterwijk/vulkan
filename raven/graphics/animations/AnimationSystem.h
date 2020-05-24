#pragma once

#include "ecs/World.h"
#include "graphics/models/SkinnedMesh.h"

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

class AnimationSystem
{
public:
	virtual void Tick( Ecs::World& World, float DeltaTimeInSeconds ) final
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

			AnimationData._Time += DeltaTimeInSeconds;

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
};