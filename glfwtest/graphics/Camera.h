#pragma once

class Camera
{
public:
	struct Buffer
	{
		glm::mat4 model = {};
		glm::mat4 view = {};
		glm::mat4 proj = {};
	};

public:

	// speed is usually 0.1f or something small like that
	void Rotate( float amount, glm::vec3& axis );

	void Translate( glm::vec3& direction );

	// call this once per loop
	// m_up is glm::vec3(0, 1, 0) for a first person camera

protected:
	void UpdateBuffer( );

protected:
	glm::vec3 _Position;
	glm::vec3 _Direction;

	Buffer _Data;
};