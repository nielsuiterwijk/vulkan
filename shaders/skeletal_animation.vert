#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraBuffer 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} camera;

layout(binding = 1) uniform Bones 
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

out gl_PerVertex 
{
    vec4 gl_Position;
};

mat4 boneTransform() 
{
  mat4 ret;

  // Weight normalization factor
  float normalizationFactor = 1.0 / (inWeights.x + inWeights.y + inWeights.z + inWeights.w);

  // Weight1 * Bone1 + Weight2 * Bone2
  ret = normalizationFactor * inWeights.w * skeletal.bones[int(inJoints.w)]
      + normalizationFactor * inWeights.z * skeletal.bones[int(inJoints.z)];
      + normalizationFactor * inWeights.y * skeletal.bones[int(inJoints.y)];
      + normalizationFactor * inWeights.x * skeletal.bones[int(inJoints.x)];

  return ret;
}

void main() {
    gl_Position = camera.proj * camera.view * camera.model * vec4(inPosition, 1.0) * boneTransform();
		
	vec4 a = camera.view * camera.model * vec4(inPosition, 1.0);
	vec4 b = camera.view * camera.model * vec4(inNormal, 0.0);
		
    vertexPosition = a.xyz;
	vertexNormal = b.xyz;
    fragColor = inColor;
	fragTexCoord = inTexCoord;
}