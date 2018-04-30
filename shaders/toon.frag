#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D _albedo;
//layout(binding = 2) uniform sampler2D _normal;
//layout(binding = 3) uniform sampler2D _specular;
//layout(binding = 4) uniform sampler2D _toonramp;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexPosition;

layout(location = 0) out vec4 outColor;


const vec3 lightPos = vec3(4.0,4.0,4.0);
const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

const vec4 rimColor = vec4(0.97, 0.88, 1, 0.75);
const float rimPower = 2.5;

void main() 
{
	vec3 normal = normalize(vertexNormal); 
	vec3 lightDir = normalize(lightPos - vertexPosition);

	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if(lambertian > 0.0) 
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		vec3 viewDir = normalize(-vertexPosition);

		float specAngle = max(dot(reflectDir, viewDir), 0.0);
		specular = pow(specAngle, 4.0);
		specular *= lambertian;
	}

	outColor = vec4(lambertian * diffuseColor + specular * specColor, 1.0);

}