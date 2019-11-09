#pragma once



struct CameraUBO
{
	glm::mat4 model = {};
	glm::mat4 view = {};
	glm::mat4 proj = {};
};

struct SkinnedMeshBuffer
{
	glm::mat4 model = {};
	glm::mat4 jointMatrix[ MAX_BONES ] = {};
	glm::vec4 lightPos = glm::vec4( 0.0f, -250.0f, 250.0f, 1.0 );
};
