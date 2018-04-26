#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(binding = 0) uniform UniformBufferObject 
{
    vec2 uScale;
    vec2 uTranslate;
} ubo;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
    fragColor = aColor;
    fragTexCoord = aUV;
    //gl_Position = vec4(aPos * ubo.uScale + ubo.uTranslate, 0, 1);
	gl_Position = vec4(aPos, 0, 1);
}