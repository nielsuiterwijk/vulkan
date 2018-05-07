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
const vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1.0);

const vec4 specColor = vec4(0.6, 0.26, 0.2, 1.0);
const float shininess = 8.0;

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
	float specularHighlight = 0.0;
 
	if(lambert > 0.0) 
	{
		vec3 viewDir = normalize(-vertexPosition);
		
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specularHighlight = pow(specAngle, shininess);
	}
	
	vec4 albedoTex = texture(_albedo, fragTexCoord);
	vec4 albedo = albedoTex * vec4(fragColor, 1.0);
	
	vec4 specular = specularHighlight * specColor;

	outColor = ambientColor + (lambert * albedo) + specular;
	outColor.a = 1;

}