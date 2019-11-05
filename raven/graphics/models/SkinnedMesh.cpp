#include "SkinnedMesh.h"
#include "graphics/buffers/UniformBuffer.h"

SkinnedMesh::SkinnedMesh()
{
	localMeshUniformBuffer = new UniformBuffer( { &skinnedMeshBuffer, sizeof( SkinnedMeshBuffer ) } );
}

SkinnedMesh::~SkinnedMesh()
{
	//Done now somewhere else (material)
	//delete localMeshUniformBuffer;
}

glm::mat4 SkinnedMesh::localMatrix( const BoneInfo& bone )
{
	return glm::translate( glm::mat4( 1.0f ), bone.translation ) * glm::toMat4( bone.rotation ) * glm::scale( glm::mat4( 1.0f ), bone.scale ) * bone.localTransform;
}

glm::mat4 SkinnedMesh::getMatrix( const BoneInfo& bone ) //Note: very efficient calculation of the bones over and over again
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

void SkinnedMesh::Update( float delta )
{
	if ( animations.size() == 0 || skins.size() == 0 )
		return;
	
	constexpr float TicksPerSecond = 25.0f; //todo read from gltf
	constexpr float Duration = 2.0f; //todo read from gltf

	const Animation& animation = animations[ 0 ];
	const SkinInfo& skin = skins[ 0 ];

	time += delta;

	while (time > Duration)
	{
		time -= Duration;
	}

	float AnimationTime = time;// (time / Duration)* TicksPerSecond;

	//Run animation
	animation.ReadNodeHierarchy( AnimationTime, skin.rootBone, bones );

	glm::mat4 rootTransform = getMatrix( bones[ skin.rootBone ] );
	glm::mat4 inverseRootTransform = glm::inverse( rootTransform );

	skinnedMeshBuffer.model = rootTransform;

	glm::mat4 jointMatrix;

	for ( int32_t i = 0; i < skin.joints.size(); i++ )
	{
		int32_t jointIndex = skin.joints[ i ];
		const BoneInfo& bone = bones[ jointIndex ];

		jointMatrix = getMatrix( bone ) * skin.inverseBindMatrices[ i ];
		jointMatrix = inverseRootTransform * jointMatrix;
		skinnedMeshBuffer.jointMatrix[ i ] = jointMatrix;
	}
}
