#include "SkinnedMesh.h"

#include "graphics/buffers/UniformBuffer.h"

#include "ecs/World.h"

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
