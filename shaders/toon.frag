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


const vec3 lightPos = vec3(3.0, 3.0, 3.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

const vec4 rimColor = vec4(0.97, 0.88, 1, 0.75);
const float rimPower = 2.5;

void main() 
{
	vec3 normal = normalize(vertexNormal); 
	vec3 lightDir = normalize(lightPos - vertexPosition);
	
	float distance = length(lightPos - vertexPosition);
	
    // Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
    // pointing in the same direction then it will get max illumination.
    float lambert = max(dot(normal, lightDir), 0.1);
	float diffuse = lambert * (1.0 / (1.0 + (0.25 * distance * distance)));
	float specular = 0.0;
 
	if(lambert > 0.0) 
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		vec3 viewDir = normalize(-vertexPosition);

		float specAngle = max(dot(reflectDir, viewDir), 0.0);
		specular = pow(specAngle, 40.0); //The higher pow number, the shinier the object becomes
		specular *= lambert;
	}

	//outColor = vec4(lambertian * diffuseColor + specular * specColor, 1.0);
	outColor = vec4(fragColor * lambert + specular * specColor, 1.0);

}