#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 0) uniform CameraBuffer 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} camera;

layout(set=0, binding = 1) uniform Bones 
{
    mat4 model;
    mat4 bones[64];
    vec4 lightPos;
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
  //ret = normalizationFactor * inWeights.w * skeletal.bones[int(inJoints.w)]
  //    + normalizationFactor * inWeights.z * skeletal.bones[int(inJoints.z)];
  //    + normalizationFactor * inWeights.y * skeletal.bones[int(inJoints.y)];
  //    + normalizationFactor * inWeights.x * skeletal.bones[int(inJoints.x)];
  
  ret = normalizationFactor * inWeights.x * skeletal.bones[int(inJoints.x)];
      + normalizationFactor * inWeights.y * skeletal.bones[int(inJoints.y)];
      + normalizationFactor * inWeights.z * skeletal.bones[int(inJoints.z)];
      + normalizationFactor * inWeights.w * skeletal.bones[int(inJoints.w)];
	  
	return ret;
}

void main() 
{
	mat4 cameraTransform = camera.view * camera.model;
	//mat4 boneTransform = mat4(1.0);
	mat4 boneTransform = boneTransform();
	
    vec4 newVertex = cameraTransform * boneTransform * vec4(inPosition, 1.0);
    vec4 newNormal = cameraTransform * boneTransform * vec4(inNormal, 0.0);
	
	newVertex.w = 1.0;
	newNormal.w = 0.0;
	
    gl_Position = camera.proj * newVertex;
		
	vec4 a = newVertex;
	vec4 b = newNormal;
		
    vertexPosition = a.xyz;
	vertexNormal = b.xyz;
    fragColor = inColor;
	fragTexCoord = inTexCoord;
}