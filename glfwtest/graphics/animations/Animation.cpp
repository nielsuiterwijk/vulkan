#include "Animation.h"

//#include "helpers/Utility.h"
#include "graphics/models/SkinnedMesh.h"

// Get node hierarchy for current animation time
void Animation::ReadNodeHierarchy( float AnimationTime, int32_t targetBone, std::vector<BoneInfo>& bones ) const
{
	BoneInfo& boneInfo = bones[ targetBone ];

	const BoneAnimation& boneAnimation = FindIf( boneAnimationFrames, [targetBone]( const BoneAnimation& boneAnimation ) { return boneAnimation.targetBone == targetBone; } );

	boneInfo.scale = interpolateScale( AnimationTime, boneAnimation );
	boneInfo.rotation = interpolateRotation( AnimationTime, boneAnimation );
	boneInfo.translation = interpolateTranslation( AnimationTime, boneAnimation );

	for ( uint32_t i = 0; i < boneInfo.children.size(); i++ )
	{
		ReadNodeHierarchy( AnimationTime, boneInfo.children[ i ], bones );
	}
}

int32_t Animation::CalculateFrame( float time, const BoneAnimation& boneAnimation ) const
{
	uint32_t frameIndex = 0;
	for ( uint32_t i = 0; i < boneAnimation.keyFrames.size() - 1; i++ )
	{
		if ( time < (float)boneAnimation.keyFrames[ i + 1 ] )
		{
			frameIndex = i;
			break;
		}
	}

	return frameIndex;
}
// Returns a 4x4 matrix with interpolated translation between current and next frame
glm::vec3 Animation::interpolateTranslation( float time, const BoneAnimation& boneAnimation ) const
{
	glm::vec3 translation;

	if ( boneAnimation.keyFrames.size() == 1 )
	{
		translation = boneAnimation.translations[ 0 ];
	}
	else
	{
		int32_t frameIndex = CalculateFrame( time, boneAnimation );
		int32_t nextFrameIndex = ( frameIndex + 1 ) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[ frameIndex ];
		float nextTime = boneAnimation.keyFrames[ nextFrameIndex ];

		const glm::vec3& start = boneAnimation.translations[ frameIndex ];
		const glm::vec3& end = boneAnimation.translations[ nextFrameIndex ];

		float delta = ( time - startTime ) / ( nextTime - startTime );

		translation = glm::lerp( start, end, delta );
	}

	return translation;
}
glm::vec3 Animation::interpolateScale( float time, const BoneAnimation& boneAnimation ) const
{
	glm::vec3 scale;

	if ( boneAnimation.keyFrames.size() == 1 )
	{
		scale = boneAnimation.scales[ 0 ];
	}
	else
	{
		int32_t frameIndex = CalculateFrame( time, boneAnimation );
		int32_t nextFrameIndex = ( frameIndex + 1 ) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[ frameIndex ];
		float nextTime = boneAnimation.keyFrames[ nextFrameIndex ];

		const glm::vec3& start = boneAnimation.scales[ frameIndex ];
		const glm::vec3& end = boneAnimation.scales[ nextFrameIndex ];

		float delta = ( time - startTime ) / ( nextTime - startTime );

		scale = glm::lerp( start, end, delta );
	}

	return scale;
}

// Returns a 4x4 matrix with interpolated rotation between current and next frame
glm::quat Animation::interpolateRotation( float time, const BoneAnimation& boneAnimation ) const
{
	glm::quat rotation;

	if ( boneAnimation.keyFrames.size() == 1 )
	{
		rotation = boneAnimation.rotations[ 0 ];
	}
	else
	{
		int32_t frameIndex = CalculateFrame( time, boneAnimation );
		int32_t nextFrameIndex = ( frameIndex + 1 ) % boneAnimation.keyFrames.size();

		float startTime = boneAnimation.keyFrames[ frameIndex ];
		float nextTime = boneAnimation.keyFrames[ nextFrameIndex ];

		const glm::quat& start = boneAnimation.rotations[ frameIndex ];
		const glm::quat& end = boneAnimation.rotations[ nextFrameIndex ];

		float delta = ( time - startTime ) / ( nextTime - startTime );

		rotation = glm::normalize( glm::slerp( start, end, delta ) );
	}

	return rotation;
}
