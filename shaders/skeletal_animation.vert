#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "camera.vert_include"

layout(set=0, binding = 1) uniform Bones 
{
    mat4 rootTransform;
    mat4 bones[64];
} skeletal;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec3 inNormal;

layout(location = 4) in vec4 inJoints;
layout(location = 5) in vec4 inWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 vertexNormal;
layout(location = 3) out vec3 vertexPosition;
layout(location = 4) out vec2 debugInfo;

out gl_PerVertex 
{
    vec4 gl_Position;
};

mat4 boneTransform() 
{
	mat4 ret;

	// Weight normalization factor
	float normalizationFactor = 1.0 / (inWeights.x + inWeights.y + inWeights.z + inWeights.w);
	debugInfo.x = normalizationFactor;

	// Weight1 * Bone1 + Weight2 * Bone2  
	ret = normalizationFactor * inWeights.x * skeletal.bones[int(inJoints.x)];
	ret += normalizationFactor * inWeights.y * skeletal.bones[int(inJoints.y)];
	ret += normalizationFactor * inWeights.z * skeletal.bones[int(inJoints.z)];
	ret += normalizationFactor * inWeights.w * skeletal.bones[int(inJoints.w)];
	  
	return ret;
}

void main() 
{
	mat4 boneTransform = boneTransform();

	vec4 locPos = camera.model * skeletal.rootTransform * boneTransform * vec4(inPosition, 1.0);	
	
	vertexPosition = locPos.xyz / locPos.w;	
	mat3 baseVertexNormal = mat3(camera.model * skeletal.rootTransform * boneTransform);
	mat3 inverseVertexNormal = inverse( baseVertexNormal );
	vertexNormal = normalize( transpose( inverseVertexNormal ) * inNormal);
	
    fragColor = inColor;
	fragTexCoord = inTexCoord;	
	
	gl_Position =  camera.proj * camera.view * vec4(vertexPosition, 1.0);
}