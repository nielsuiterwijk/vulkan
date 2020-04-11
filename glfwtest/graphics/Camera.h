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

	static constexpr glm::vec3 Up = { 0, 1, 0 };
	static constexpr glm::vec3 Forward = { 0, 0, -1 };
	static constexpr glm::vec3 Right = { 1, 0, 0 };

public:

	void CalculateProjection( float AngleInDegrees );

	void CalculateOrientation( float HorizontalAngleInDegrees, float VerticalAngleIndegrees );


	void Translate( const glm::vec3& direction );

	glm::vec3 CalculateForward() const;
	glm::vec3 CalculateRight() const;

	// call this once per loop
	// m_up is glm::vec3(0, 1, 0) for a first person camera

	const Buffer& Data() const { return _Data; }

protected:
	void CalculateView( );

protected:
	glm::vec3 _Position = glm::vec3( 2, 1, 2 );
	glm::quat _Orientation = {};//glm::vec3( -2, -1, -2 );

	float ZNear = 0.01f;
	float ZFar = 100.0f;
	float AspectRatio = 16.0f / 9.0f;
		

	Buffer _Data;
};