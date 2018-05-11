#pragma once

#include "standard.h"

class AABB
{
public:
	explicit AABB() :
		centre(0, 0, 0),
		extents(0, 0, 0)
	{

	};
	~AABB() {};

	void Grow(const glm::vec3& newPoint)
	{
		glm::vec3 delta = newPoint - centre;

		if (std::abs(delta.x) > extents.x)
		{
			extents.x += std::abs(delta.x / 2.0f);
			centre.x += delta.x / 2.0f;
		}

		if (std::abs(delta.y) > extents.y)
		{
			extents.y += std::abs(delta.y / 2.0f);
			centre.y += delta.y / 2.0f;
		}

		if (std::abs(delta.z) > extents.z)
		{
			extents.z += std::abs(delta.z / 2.0f);
			centre.z += delta.z / 2.0f;
		}
		
	}

	bool Overlaps(const AABB& other)
	{
		if (std::abs(centre.x - other.centre.x) > (extents.x + other.extents.x)) return false;
		if (std::abs(centre.y - other.centre.y) > (extents.y + other.extents.y)) return false;
		if (std::abs(centre.z - other.centre.z) > (extents.z + other.extents.z)) return false;

		return false;
	}

private:
	glm::vec3 centre;
	glm::vec3 extents;
};